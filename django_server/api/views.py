import base64
import json
import re
import urllib.parse
from time import sleep

import requests
import shortuuid
import uuid
from celery.bin.control import inspect
from celery.exceptions import TaskRevokedError
from celery.result import AsyncResult
from django.http import JsonResponse, HttpResponseNotFound
from django.shortcuts import render
from django.views.decorators.csrf import csrf_exempt
from django.conf import settings
from rest_framework.authentication import TokenAuthentication
from rest_framework.decorators import authentication_classes, api_view, permission_classes
from rest_framework.permissions import IsAuthenticated, AllowAny
import vk_api

from api.models import UserSocial, GroupBot, GroupAccessCode, BotTask
from api.tasks import startBot
from django_celery_results.models import TaskResult

from celery.backends.redis import RedisBackend

from django_server import celery
from django_server.celery import app, CeleryTaskManager

messages = {
    "errors": {
        "UNDEFINED": {
            "status": "error",
            "code": 233,
            "error_msg": "One of the specified parameters is missing or invalid",
        },
        "DUPLICATE": {
            "status": "error",
            "code": 235,
            "error_msg": "Record with this field already exists",
        },
        "CODE": {
            "status": "error",
            "code": 231,
            "error_msg": "Code is invalid or expired",
        },
        "NOTFOUND": {
            "status": "error",
            "code": 404,
            "error_msg": "Not found",
        }
    },

    "success": {
        "DELETE": {
            "status": "success",
            "success_msg": "The data was successfully deleted",
        },
        "CREATE": {
            "status": "success",
            "success_msg": "The data was successfully added",
        }
    }
}

AUTHORIZATION_URL = 'https://oauth.vk.com/authorize?'


def error_404(request, exception):
    if request.method == "POST":
        return JsonResponse(createJsonError(messages["errors"]["NOTFOUND"]))
    else:
        return HttpResponseNotFound("<h1>Страница не найдена 404</h1>")


def createJsonError(obj):
    error = {
        "error": {
            "error_code": obj["code"],
            "error_msg": obj["error_msg"]
        }
    }
    return error


# ________________Декораторы_для_доступа_______________ #

def allow_any_perm(func):
    @api_view(['POST'])
    @authentication_classes([TokenAuthentication])
    @permission_classes([AllowAny])
    def wrapper(request, *args, **kwargs):
        return func(request, *args, **kwargs)

    return wrapper


def is_auth_perm(func):
    @api_view(['POST'])
    @authentication_classes([TokenAuthentication])
    @permission_classes([IsAuthenticated])
    def wrapper(request, *args, **kwargs):
        return func(request, *args, **kwargs)

    return wrapper


# ____________________________________________________________ #


@allow_any_perm
def urlOauth(request):
    if request.method == "POST":
        id = getattr(settings, 'AUTH_VK_OAUTH2_ID', None)
        params = {'client_id': id,
                  'redirect_uri': 'http://127.0.0.1:8000/api/auth/code/',
                  'response_type': 'code',
                  'v': '5.131',
                  'scope': 'friend, photos, audio, phone_number, email, groups, stats, notify'}
        url = AUTHORIZATION_URL + urllib.parse.urlencode(params)
        return JsonResponse({'url_oauth': url})


@is_auth_perm
def urlGroupOauth(request):
    group_id = request.POST.get("id")
    if group_id is None:
        return JsonResponse(createJsonError(messages["errors"]["UNDEFINED"]))

    length = GroupAccessCode._meta.get_field("access_code").max_length
    access_code = shortuuid.uuid()[:length]

    GroupAccessCode.create_or_update(group_id, access_code, request.user)

    id = getattr(settings, 'AUTH_VK_OAUTH2_ID', None)
    params = {'client_id': id,
              'scope': 'messages, docs, manage, photos',
              'redirect_uri': 'http://127.0.0.1:8000/api/bot/create/?access_code=' + access_code,
              'response_type': 'code',
              'group_ids': group_id,
              'v': '5.131'}
    url = AUTHORIZATION_URL + urllib.parse.urlencode(params)
    return JsonResponse({'url_oauth': url})


@is_auth_perm
def getAdminGroups(request):
    access_token = UserSocial.objects.get(user_id=request.user.id).access_token
    vk_session = vk_api.VkApi(token=access_token)
    vk = vk_session.get_api()
    admin_groups = vk.groups.get(filter='admin', extended=False)
    groups = []
    for group_id in admin_groups['items']:
        group_info = vk.groups.getById(group_id=group_id, fields="name,photo_200")[0]
        if not GroupBot.objects.filter(id=group_info['id']).exists():
            photo = ''
            response = requests.get(group_info['photo_200'])
            if response.status_code == 200:
                photo = "data:image/jpeg;base64," + base64.b64encode(response.content).decode('utf-8')

            groups.append({
                'id': group_info['id'],
                'name': group_info['name'],
                'photo': photo
            })
    return JsonResponse(groups, safe=False, json_dumps_params={'ensure_ascii': False})


def code(request):
    if request.method == "GET":
        code = request.GET.get("code", None)
        return render(request, 'code.html', {'code': code})


@is_auth_perm
def start(request):
    group_id = request.POST.get('id')
    if not group_id:
        return JsonResponse(createJsonError(messages["errors"]["UNDEFINED"]))
    bot = GroupBot.objects.filter(id=group_id).first()
    if not bot:
        return JsonResponse(createJsonError(messages["errors"]["NOTFOUND"]))
    unique_id = str(uuid.uuid4()) + '-' + str(bot.id)
    bot_task = BotTask.objects.filter(bot=bot).first()
    if bot_task is not None:
        celery_manage = CeleryTaskManager()
        if celery_manage.is_active_task(bot_task.task_id):
            return JsonResponse({'error': 'Bot is running'})
    startBot.apply_async(args=[bot.token, bot.id], task_id=unique_id)
    BotTask.create_or_update(bot.id, unique_id)
    return JsonResponse({'success': 'Bot starting'})


@is_auth_perm
def stop(request):
    group_id = request.POST.get('id')
    if not group_id:
        return JsonResponse(createJsonError(messages["errors"]["UNDEFINED"]))

    bot = GroupBot.objects.filter(id=group_id).first()
    if not bot:
        return JsonResponse(createJsonError(messages["errors"]["NOTFOUND"]))

    bot_task = BotTask.objects.filter(bot=bot).first()
    if bot_task is not None:
        celery_manage = CeleryTaskManager()
        if celery_manage.is_active_task(bot_task.task_id):
            celery_manage.delete_task(bot_task.task_id)
            return JsonResponse({'success': 'Bot stopping'})

    return JsonResponse({'error': 'Bot is stopping'})


def commands(request):
    return JsonResponse({'success': 'Commands create and save'})


def getCommands(request):
    return JsonResponse({'success': 'Commands'})

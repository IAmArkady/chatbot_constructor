import json
import re

import requests
from django.http import JsonResponse
from django.shortcuts import render
from rest_framework.authentication import TokenAuthentication
from rest_framework.generics import ListAPIView
from rest_framework.permissions import IsAuthenticated, AllowAny
from rest_framework.response import Response
from rest_framework.views import APIView
from django.conf import settings

from api.models import GroupBot, GroupAccessCode, BotTask, GroupBotCommands, UserCurrentCommand
from api.serializers import BotsSerializer
from api.views import messages, createJsonError
from django_server.celery import app, CeleryTaskManager


class BotsList(ListAPIView):
    serializer_class = BotsSerializer
    authentication_classes = [TokenAuthentication]
    permission_classes = [IsAuthenticated]

    def get_user_queryset(self, user):
        return GroupBot.objects.all().filter(user=user)

    def get(self, request, *args, **kwargs):
        queryset = self.get_user_queryset(request.user)
        serializer = self.serializer_class(queryset, many=True, context={'request': request})
        task_manage = CeleryTaskManager()
        for i in range(0, queryset.count()):
            active = False
            bot_task = BotTask.objects.filter(bot=queryset[i]).first()
            if bot_task is not None:
                active = task_manage.is_active_task(bot_task.task_id)
            serializer.data[i]['status'] = active
        return Response(serializer.data)


class BotsCreate(APIView):
    authentication_classes = [TokenAuthentication]
    permission_classes = [AllowAny]

    def get(self, request):
        access_code = request.GET.get("access_code", None)
        code = request.GET.get('code', None)

        try:
            group_code = GroupAccessCode.objects.get(access_code=access_code)
        except GroupAccessCode.DoesNotExist:
            return render(request, 'token.html', {'error': 'Ошибка, код недействителен'})

        ACCESS_TOKEN_URL = 'https://oauth.vk.com/access_token'
        id = getattr(settings, 'AUTH_VK_OAUTH2_ID', None)
        secret = getattr(settings, 'AUTH_VK_OAUTH2_KEY_SECRET', None)
        params = {'client_id': id,
                  'client_secret': secret,
                  'code': code,
                  'redirect_uri': 'http://127.0.0.1:8000/api/bot/create/?access_code=' + access_code}

        response = requests.post(ACCESS_TOKEN_URL, params)
        if response.status_code == 200:
            info_group = response.json()['groups'][0]
            GroupBot.create_or_update(info_group['group_id'], info_group['access_token'], group_code.user)
            group_code.delete()
        else:
            return render(request, 'token.html', {'error': 'Ошибка, код ВК недействителен'})

        return render(request, 'token.html', {'success': 'Бот успешно подключен'})


class BotsDelete(APIView):
    authentication_classes = [TokenAuthentication]
    permission_classes = [IsAuthenticated]

    def delete_object(self, id, user):
        try:
            obj = GroupBot.objects.get(id=id, user=user)
            obj.delete()
            return True
        except GroupBot.DoesNotExist:
            return False

    def post(self, request):
        id = request.POST.get('id')
        if not id:
            return JsonResponse(createJsonError(messages["errors"]["UNDEFINED"]))

        deleted = self.delete_object(id=id, user=request.user)
        if deleted:
            return JsonResponse(messages["success"]["DELETE"])
        else:
            return JsonResponse(messages["errors"]["NOTFOUND"])


class CommandsCreate(APIView):
    authentication_classes = [TokenAuthentication]
    permission_classes = [IsAuthenticated]

    def json_keywords_command(self, command):
        reg = command["current_index"]
        keywords_text = command["keywords_text"]
        if keywords_text:
            if re.search(r"равно", reg):
                keywords_text = '^' + keywords_text + '$'
            elif re.search(r"содержит", reg):
                keywords_text = keywords_text
            elif re.search(r"начинается", reg):
                keywords_text = '^' + keywords_text

        json_data = {
            "command_info": {"keywords_text": keywords_text},
            "command_next": {"command_next": command["next_command"] if "next_command" in command else None},
        }
        return json_data

    def json_actions_command(self, command):
        json_data = {
            "command_info": {"variables": command["variables"]},
            "command_next": {"command_next": command["next_command"] if "next_command" in command else None},
        }
        return json_data

    def json_inputs_command(self, command):
        json_data = {
            "command_info": {"variable": command["variable"]},
            "command_next": {"command_next": command["next_command"] if "next_command" in command else None},
        }
        return json_data

    def json_messages_command(self, command):
        button_command_next = []
        all_buttons = []
        if "buttons" in command:
            for button_block in command["buttons"]:
                buttons = []
                for button in button_block:
                    buttons.append(button)
                    button_command_next.append(
                        {"command_next": button["next_command"] if "next_command" in button else None,
                         "payload": button["payload"]})
                all_buttons.append(buttons)

        json_data = {
            "command_info": {"messages_text": command["messages_text"],
                             "buttons": all_buttons},
            "command_next": {"command_next": command["next_command"] if "next_command" in command else None,
                             "event": button_command_next},
        }
        return json_data

    def json_requests_command(self, command):
        buttons = command["buttons"]
        success = {"command_next": buttons[0]["next_command"] if "next_command" in buttons[0] else None}
        errors = {"command_next": buttons[1]["next_command"] if "next_command" in buttons[1] else None}
        event_command_next = {"success": success,
                              "errors": errors}

        json_data = {
            "command_info": {"type": command["type"],
                             "link": command["link"],
                             "head": command["head"],
                             "body": command["body"],
                             "save_variable": command["save_variable"]},
            "command_next": {"command_next": command["next_command"] if "next_command" in command else None,
                             "event": event_command_next},
        }
        return json_data

    def post(self, request):
        id = request.POST.get('id')
        commands = request.POST.get('commands')
        if not id or not commands:
            return JsonResponse(createJsonError(messages["errors"]["UNDEFINED"]))

        GroupBot.update_commands(id, commands)
        GroupBotCommands.delete_commands_by_bot(bot_id=id)
        UserCurrentCommand.delete_commands_by_bot(bot_id=id)

        data = json.loads(commands)
        commands = data['commands']
        command_info = ""
        command_next = ""
        for command in commands:
            if command["type_command"] == "keywords":
                temp = self.json_keywords_command(command)
                command_info = temp["command_info"]
                command_next = temp["command_next"]

            elif command["type_command"] == "messages":
                temp = self.json_messages_command(command)
                command_info = temp["command_info"]
                command_next = temp["command_next"]

            elif command["type_command"] == "requests":
                temp = self.json_requests_command(command)
                command_info = temp["command_info"]
                command_next = temp["command_next"]

            elif command["type_command"] == "inputs":
                temp = self.json_inputs_command(command)
                command_info = temp["command_info"]
                command_next = temp["command_next"]

            elif command["type_command"] == "actions":
                temp = self.json_actions_command(command)
                command_info = temp["command_info"]
                command_next = temp["command_next"]

            GroupBotCommands.create(
                command_id=command["command"],
                command_next=command_next,
                command_type=command["type_command"],
                command_info=command_info,
                bot_id=id
            )

        return JsonResponse(messages["success"]["CREATE"])

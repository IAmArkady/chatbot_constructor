import requests
from django.contrib.auth import authenticate
from django.http import JsonResponse
from rest_framework.authentication import TokenAuthentication
from rest_framework.permissions import AllowAny, IsAuthenticated
from rest_framework.views import APIView
from api.views import messages
from django_server import settings
from rest_framework.authtoken.models import Token
from rest_framework.response import Response


class SocLogout(APIView):
    authentication_classes = [TokenAuthentication]
    permission_classes = [IsAuthenticated]

    def post(self, request):
        request.user.auth_token.delete()
        return Response(status=204)


class SocLogin(APIView):
    authentication_classes = [TokenAuthentication]
    permission_classes = [AllowAny]

    def post(self, request):
        ACCESS_TOKEN_URL = 'https://oauth.vk.com/access_token'
        code = request.POST.get("code", None)
        if not code:
            return JsonResponse(messages["errors"]["UNDEFINED"])

        id = getattr(settings, 'AUTH_VK_OAUTH2_ID', None)
        secret = getattr(settings, 'AUTH_VK_OAUTH2_KEY_SECRET', None)
        params = {'client_id': id,
                  'client_secret': secret,
                  'code': code,
                  'redirect_uri': 'http://127.0.0.1:8000/api/auth/code/'}
        response = requests.post(ACCESS_TOKEN_URL, params)
        if response.status_code == 200:
            vk_access_token = response.json()
            vk_user_url = 'https://api.vk.com/method/users.get'
            vk_params = {
                'access_token': vk_access_token.get('access_token'),
                'fields': 'id,first_name,last_name,email,domain',
                'v': '5.131',
            }
            vk_user = requests.post(vk_user_url, params=vk_params).json()['response'][0]
            data = {'id': vk_user['id'],
                    'first_name': vk_user['first_name'],
                    'last_name': vk_user['last_name'],
                    'access_token': vk_access_token['access_token'],
                    'email': vk_access_token.get('email', None),
                    'domain': vk_user['domain']}
            user = authenticate(request, vk_user=data)
            if user is not None:
                token, created = Token.objects.get_or_create(user=user)
                return JsonResponse({'token': token.key})
        return JsonResponse(messages["errors"]["CODE"])

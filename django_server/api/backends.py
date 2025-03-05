import random
import string

from django.contrib.auth.backends import BaseBackend
from django.contrib.auth.models import User

from api.models import UserSocial


class VkCodeAuthentication(BaseBackend):
    def authenticate(self, request, vk_user=None):
        try:
            user = User.objects.get(email=vk_user['email'])
            user.username = vk_user['domain']
            user.first_name = vk_user['first_name']
            user.last_name = vk_user['last_name']
            user.save()

            UserSocial.create_or_update(
                id=vk_user['id'],
                access_token=vk_user['access_token'],
                user=user
            )
            return user
        except User.DoesNotExist:
            pass

        user = User.objects.create_user(
            username=vk_user['domain'],
            email=vk_user['email'],
            first_name=vk_user['first_name'],
            last_name=vk_user['last_name'],
            password=''.join([random.choice(string.digits + string.ascii_letters) for i in range(0, 10)]),
        )

        UserSocial.create_or_update(
            id=vk_user['id'],
            access_token=vk_user['access_token'],
            user=user
        )

        return user

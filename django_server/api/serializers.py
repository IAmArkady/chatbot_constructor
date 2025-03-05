import base64

import requests
from rest_framework import serializers
import vk_api
from api.models import GroupBot, UserSocial


class BotsSerializer(serializers.ModelSerializer):
    photo = serializers.SerializerMethodField()
    name = serializers.SerializerMethodField()

    class Meta:
        model = GroupBot
        fields = ['id', 'commands', 'created_at', 'photo', 'name']

    def get_vk_api(self):
        try:
            user_social = UserSocial.objects.get(user_id=self.context['request'].user.id)
            vk_session = vk_api.VkApi(token=user_social.access_token)
            return vk_session.get_api()
        except UserSocial.DoesNotExist:
            return None

    def get_group_info(self, obj):
        vk = self.get_vk_api()
        if not vk:
            return None
        try:
            group = vk.groups.getById(group_id=obj.id)
            return group[0] if group else None
        except Exception as e:
            return None

    def get_photo(self, obj):
        group = self.get_group_info(obj)
        if not group:
            return ''
        url = group.get('photo_200', '')
        if not url:
            return ''
        response = requests.get(url)
        if response.status_code != 200:
            return ''
        encoded_image = base64.b64encode(response.content).decode('utf-8')
        return f"data:image/jpeg;base64,{encoded_image}"

    def get_name(self, obj):
        group = self.get_group_info(obj)
        return group.get('name', '') if group else ''


import json
import re
from datetime import datetime
from time import sleep

import requests
from celery.contrib.abortable import AbortableTask
from celery import shared_task
import vk_api
from vk_api import VkApiError
from vk_api.bot_longpoll import VkBotLongPoll, VkBotEventType
from vk_api.utils import get_random_id
from vk_api.keyboard import VkKeyboard, VkKeyboardColor

from api.models import GroupBot, BotTask, UserCurrentCommand, GroupBotCommands
from django_server.celery import app

class vkBot:
    def __init__(self, token, group_id):
        self.vk_session = vk_api.VkApi(token=token)
        self.vk = self.vk_session.get_api()
        self.group_id = group_id
        self.bot_longpoll = VkBotLongPoll(self.vk_session, self.group_id)

    def parse_json_varialbes(self, text, variables):
        for key, value in variables.items():
            if isinstance(value, dict):
                text = self.parse_json_varialbes(text, value)
            else:
                text = text.replace("{" + key + "}", str(value))
        return text

    def parse_text(self, text, variables):
        if not variables:
            return text

        text = self.parse_json_varialbes(text, variables)
        return text

    def make_buttons(self, buttons):
        keyboard_color = {"secondary": VkKeyboardColor.SECONDARY,
                          "negative": VkKeyboardColor.NEGATIVE,
                          "positive": VkKeyboardColor.POSITIVE,
                          "primary": VkKeyboardColor.PRIMARY}
        keyboard = VkKeyboard(one_time=True)
        for i, buttons_block in enumerate(buttons):
            for button in buttons_block:
                if button["type"] == "text":
                    keyboard.add_button(label=button["title"], color=keyboard_color[button["color"]], payload={"button": button["payload"]})
                elif button["type"] == "link":
                    keyboard.add_openlink_button(label=button["title"], link=button["link"])
            if i < len(buttons) - 1:
                keyboard.add_line()
        return keyboard

    def generate_messages(self, user_id, messages_command):
        next_command = ""
        if not messages_command.command_next["command_next"] and not messages_command.command_next["event"]:
            UserCurrentCommand.create_or_update_for_bot(user_id=user_id, command=None, bot_id=messages_command.bot_id)
            next_command = None
        else:
            UserCurrentCommand.create_or_update_for_bot(user_id=user_id, command=messages_command, bot_id=messages_command.bot_id)
            next_command = messages_command
        bot_messages = messages_command.command_info["messages_text"]
        keyboard = ""
        if messages_command.command_info["buttons"]:
            keyboard = self.make_buttons(messages_command.command_info["buttons"])
        try:
            variables = UserCurrentCommand.get_for_bot(user_id=user_id, bot_id=messages_command.bot_id).variables
            bot_messages = self.parse_text(bot_messages, variables)
            self.vk.messages.send(user_id=user_id, message=bot_messages, random_id=get_random_id(), keyboard=keyboard.get_keyboard() if keyboard else "", payload=None)
        except VkApiError as error:
            print("Ошибка: ", error)
        return next_command

    def parse_inputs(self, user_id, group_id, message_text, inputs_command):
        variable = inputs_command.command_info["variable"]
        UserCurrentCommand.add_or_update_variables_for_bot(user_id=user_id, bot_id=group_id, variable=variable, value=message_text)
        next_command = GroupBotCommands.get_by_command_id(command_id=inputs_command.command_next["command_next"], bot_id=group_id)
        return next_command

    def parse_actions(self, user_id, group_id, actions_command):
        variables = actions_command.command_info["variables"]
        for variable in variables:
            UserCurrentCommand.add_or_update_variables_for_bot(user_id=user_id, bot_id=group_id, variable=variable["variable"], value=variable["value"])

    def parse_requests(self, user_id, group_id, requests_command):
        variables = UserCurrentCommand.get_for_bot(user_id=user_id, bot_id=group_id).variables
        params = self.parse_text(requests_command.command_info["body"], variables)
        params = str(params).replace('\n', '&')
        params = params.encode()
        head_str = requests_command.command_info["head"]
        headers = {}
        for param in head_str.split('\n'):
            if param:
                key, value = param.split(': ')
                headers[key] = value.strip()
        response = requests.post(requests_command.command_info["link"], data=params, headers=headers)
        if response.status_code == 200:
            command_next_id = requests_command.command_next["event"]["success"]["command_next"]
        else:
            command_next_id = requests_command.command_next["event"]["errors"]["command_next"]
        variable = requests_command.command_info["save_variable"]
        if variable:
            UserCurrentCommand.add_or_update_variables_for_bot(user_id=user_id, bot_id=group_id, variable=variable, value=response.json())
        next_command = GroupBotCommands.get_by_command_id(command_id=command_next_id, bot_id=group_id)
        print(command_next_id)
        print(response.status_code)
        return next_command

    def parse_next_command(self, user_id, group_id, message, current_command_id):
        message_text = message.get("text")
        current_command = GroupBotCommands.get_by_command_id(command_id=current_command_id, bot_id=group_id)
        if not current_command:
            return None

        next_command_id = current_command.command_next["command_next"]
        next_command = GroupBotCommands.get_by_command_id(command_id=next_command_id, bot_id=group_id)
        if current_command.command_type == "messages":
            if "payload" in message:
                for button in current_command.command_next["event"]:
                    if button["payload"] == json.loads(message.get("payload"))["button"]:
                        next_command = GroupBotCommands.get_by_command_id(command_id=button["command_next"], bot_id=group_id)
                        break
            if next_command and next_command.command_type == "inputs":
                next_command =self.parse_inputs(user_id=user_id, group_id=group_id, message_text=message_text, inputs_command=next_command)

        elif current_command.command_type == "inputs":
            next_command = self.parse_inputs(user_id=user_id, group_id=group_id, message_text=message_text, inputs_command=current_command)

        elif current_command.command_type == "actions":
            self.parse_actions(user_id=user_id, group_id=group_id, actions_command=current_command)

        elif current_command.command_type == "requests":
            next_command = self.parse_requests(user_id=user_id, group_id=group_id, requests_command=current_command)


        UserCurrentCommand.create_or_update_for_bot(user_id=user_id, command=next_command, bot_id=group_id)
        if next_command:
            if next_command.command_type == "messages":
                next_command = self.generate_messages(user_id=user_id, messages_command=next_command)
            else:
                while next_command and next_command.command_type != "messages" and next_command.command_type != "inputs":
                    next_command = self.parse_next_command(user_id=user_id, group_id=group_id, message=message, current_command_id=next_command.command_id)
                UserCurrentCommand.create_or_update_for_bot(user_id=user_id, command=next_command, bot_id=group_id)

        return next_command

    def parse_keywords_command(self, user_id, group_id, message):
        message_text = message.get("text")
        keywords_commands = GroupBotCommands.objects.filter(bot_id=group_id, command_type="keywords")
        for keywords_record in keywords_commands:
            keywords_text = keywords_record.command_info["keywords_text"]
            regex_text = re.compile(keywords_text, re.IGNORECASE)
            if regex_text.search(message_text):
                next_command = GroupBotCommands.get_by_command_id(command_id=keywords_record.command_next["command_next"], bot_id=group_id)
                if next_command:
                    UserCurrentCommand.create_or_update_for_bot(user_id=user_id, command=next_command, bot_id=group_id)
                    if next_command.command_type == "messages" or next_command.command_type == "inputs":
                        if next_command.command_type == "messages":
                            self.generate_messages(user_id=user_id, messages_command=next_command)
                    else:
                        self.parse_next_command(user_id=user_id, group_id=group_id, message=message, current_command_id=next_command.command_id)
                break

    def process_message(self, event):
        if event.type == VkBotEventType.MESSAGE_NEW and event.from_user:
            group_id = event.group_id
            message = event.obj['message']
            user_id = message['from_id']

            current_command_user = UserCurrentCommand.get_for_bot(user_id=user_id, bot_id=group_id)
            if not current_command_user or not current_command_user.command_id:
                self.parse_keywords_command(user_id=user_id, group_id=group_id, message=message)
            else:
                command_bot = GroupBotCommands.get_by_id(current_command_user.command_id)
                self.parse_next_command(user_id=user_id, group_id=group_id, message=message, current_command_id=command_bot.command_id)

    def start(self):
        for event in self.bot_longpoll.listen():
            self.process_message(event)


@shared_task(bind=True, idempotent=True)
def startBot(self, token, group_id):
    bot = vkBot(token, group_id)
    bot.start()
    return {'msg': 'okey'}
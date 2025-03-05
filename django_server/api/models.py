from django.db import models


class GroupAccessCode(models.Model):
    access_code = models.CharField(max_length=15, unique=True)
    group_id = models.IntegerField(max_length=15)
    user = models.OneToOneField('auth.User', on_delete=models.CASCADE)
    modified_at = models.DateTimeField(auto_now=True)

    @classmethod
    def create_or_update(cls, group_id, access_code, user):
        try:
            code = cls.objects.get(group_id=group_id)
        except cls.DoesNotExist:
            code = cls(group_id=group_id, access_code=access_code, user=user)
            code.save()
        else:
            code.access_code = access_code
            code.save()
        return code

    class Meta:
        db_table = 'group_accesscode'


class GroupBot(models.Model):
    id = models.IntegerField(primary_key=True, serialize=True)
    user = models.ForeignKey('auth.User', on_delete=models.CASCADE)
    token = models.TextField()
    commands = models.TextField(blank=True)
    created_at = models.DateTimeField(auto_now_add=True)
    modified_at = models.DateTimeField(auto_now=True)

    @classmethod
    def create_or_update(cls, group_id, token, user):
        try:
            bot = cls.objects.get(id=group_id)
        except cls.DoesNotExist:
            bot = cls(id=group_id, token=token, user=user)
            bot.save()
        else:
            bot.token = token
            bot.save()
        return bot

    @classmethod
    def update_commands(cls, group_id, commands):
        try:
            bot = cls.objects.get(id=group_id)
            bot.commands = commands
            bot.save()
        except cls.DoesNotExist:
            pass

    class Meta:
        db_table = 'group_bot'


class GroupBotCommands(models.Model):
    bot = models.ForeignKey('GroupBot', on_delete=models.CASCADE)
    command_id = models.IntegerField()
    command_next = models.JSONField()
    command_type = models.CharField(max_length=128)
    command_info = models.JSONField()

    @classmethod
    def create(cls, command_id, command_next, command_type, command_info, bot_id):
        command = cls.objects.create(
            command_id=command_id,
            command_next=command_next if command_next is not None else None,
            command_type=command_type,
            command_info=command_info,
            bot_id=bot_id,
        )
        return command

    @classmethod
    def delete_commands_by_bot(cls, bot_id):
        cls.objects.filter(bot_id=bot_id).delete()

    @classmethod
    def get_by_id(cls, id):
        try:
            command = cls.objects.get(id=id)
        except cls.DoesNotExist:
            return None
        return command

    @classmethod
    def get_by_command_id(cls, command_id, bot_id):
        try:
            command = cls.objects.get(command_id=command_id, bot_id=bot_id)
        except cls.DoesNotExist:
            return None
        return command

    class Meta:
        db_table = 'group_bot_command'
        unique_together = ('bot', 'command_id')


class BotTask(models.Model):
    bot = models.OneToOneField('GroupBot', on_delete=models.CASCADE)
    task_id = models.CharField(max_length=256)
    created_at = models.DateTimeField(auto_now_add=True)

    @classmethod
    def create_or_update(cls, bot_id, task_id):
        try:
            bot_task = cls.objects.get(bot_id=bot_id)
        except cls.DoesNotExist:
            bot_task = cls(bot_id=bot_id, task_id=task_id)
            bot_task.save()
        else:
            bot_task.task_id = task_id
            bot_task.save()
        return bot_task

    class Meta:
        db_table = 'group_bot_task'


class UserCurrentCommand(models.Model):
    social_id = models.IntegerField()
    command = models.ForeignKey('GroupBotCommands', on_delete=models.CASCADE, null=True)
    variables = models.JSONField(null=True, blank=True)
    bot = models.ForeignKey('GroupBot', on_delete=models.CASCADE)

    @classmethod
    def create_or_update_for_bot(cls, user_id, command, bot_id):
        try:
            current_command = cls.objects.get(social_id=user_id, bot_id=bot_id)
        except cls.DoesNotExist:
            current_command = cls(social_id=user_id, command=command, bot_id=bot_id)
            current_command.save()
        else:
            current_command.command = command
            current_command.save()
        return current_command

    @classmethod
    def get_for_bot(cls, user_id, bot_id):
        try:
            current_command = cls.objects.get(social_id=user_id, bot_id=bot_id)
        except cls.DoesNotExist:
            return None
        return current_command

    @classmethod
    def get_command_by_command_id(cls, user_id, bot_id):
        try:
            command = cls.objects.get(social_id=user_id, bot_id=bot_id)
            command = GroupBotCommands.get_by_id(id=command.command_id)
        except cls.DoesNotExist:
            return None
        return command

    @classmethod
    def add_or_update_variables_for_bot(cls, user_id, bot_id, variable, value):
        try:
            current_command = cls.objects.get(social_id=user_id, bot_id=bot_id)
        except cls.DoesNotExist:
            return None
        else:
            variables = current_command.variables
            if variables:
                variables[variable] = value
            else:
                variables = {variable: value}
            current_command.variables = variables
            current_command.save()
        return current_command

    @classmethod
    def delete_commands_by_bot(cls, bot_id):
        cls.objects.filter(bot_id=bot_id).delete()

    class Meta:
        unique_together = ('bot', 'social_id')
        db_table = 'user_current_command'


class UserSocial(models.Model):
    id = models.IntegerField(primary_key=True, serialize=False)
    access_token = models.TextField()
    user = models.OneToOneField('auth.User', on_delete=models.CASCADE)
    created_at = models.DateTimeField(auto_now_add=True)
    modified_at = models.DateTimeField(auto_now=True)

    @classmethod
    def get_social_auth(cls, uid):
        try:
            soc_user = cls.objects.select_related('user').get(user_id=uid)
            return soc_user
        except cls.DoesNotExist:
            return None

    @classmethod
    def create_or_update(cls, id, access_token, user):
        try:
            soc_user = cls.objects.get(id=id)
        except cls.DoesNotExist:
            soc_user = cls(id=id, access_token=access_token, user=user)
            soc_user.save()
        else:
            soc_user.access_token = access_token
            soc_user.user = user
            soc_user.save()
        return soc_user

    class Meta:
        db_table = 'auth_user_social'

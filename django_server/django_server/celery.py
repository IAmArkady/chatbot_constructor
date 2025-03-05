from __future__ import absolute_import
import os
from celery import Celery
import socket
from celery.bin.control import inspect
from celery.result import AsyncResult

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'django_server.settings')
app = Celery("django_server")
app.config_from_object('django.conf:settings', namespace='CELERY')
app.autodiscover_tasks()
app.conf.task_default_queue = 'default'
app.conf.task_default_exchange = 'tasks'
app.conf.task_default_exchange_type = 'topic'
app.conf.task_default_routing_key = 'task.default'


class CeleryTaskManager:
    def __init__(self, celery_app=app):
        self.celery_app = celery_app

    def get_active_tasks(self):
        return inspect(app=self.celery_app).active()

    def get_pending_tasks(self):
        return inspect(app=self.celery_app).reserved()

    def is_active_task(self, task_id):
        result = AsyncResult(task_id)
        if result.state != "REVOKED":
            celery_host = 'celery@'+socket.gethostname()
            result = app.control.inspect([celery_host]).query_task(task_id)
            if result and result[celery_host]:
                return True
        return False

    def delete_task(self, task_id):
        self.celery_app.control.revoke(task_id, terminate=True)
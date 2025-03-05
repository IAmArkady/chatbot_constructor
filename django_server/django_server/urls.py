from django.contrib import admin
from django.urls import path, include

from api import views
from api.apiview import BotsList, BotsDelete, BotsCreate, CommandsCreate
from api.auth import SocLogin, SocLogout

commandpatterns = [
    path('set/', CommandsCreate.as_view()),
    path('get/', views.getCommands),
]

botpatterns = [
    path('commands/', include(commandpatterns)),
    path('create/', BotsCreate.as_view()),
    path('get/', BotsList.as_view()),
    path('delete/', BotsDelete.as_view()),
    path('start/', views.start),
    path('stop/', views.stop),
    path('adminGroups/', views.getAdminGroups),
    path('url/', views.urlGroupOauth),
]

authpatterns = [
    path('code/', views.code, name="code"),
    path('url/', views.urlOauth),
    path('login/', SocLogin.as_view()),
    path('logout/', SocLogout.as_view()),
]

apipatterns = [
    path('auth/', include(authpatterns)),
    path('bot/', include(botpatterns))
]

urlpatterns = [
    path('admin/', admin.site.urls),
    path('api/', include(apipatterns)),
    path('test/get/', views.testGet)
]



handler404 = views.error_404

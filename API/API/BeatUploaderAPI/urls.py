from django.urls import path
from BeatUploaderAPI.views import api_views, static_views

urlpatterns = [
    path('upload/', api_views.upload),
    path('getAccessToken/', api_views.getAccessToken),
    path('static/', static_views.main),
]

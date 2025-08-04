from django.urls import path
from BeatUploaderAPI import views

urlpatterns = [
    path('upload/', views.upload),
    path('getAccessToken/', views.getAccessToken),
]

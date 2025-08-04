from rest_framework.decorators import api_view
from rest_framework.response import Response
from rest_framework import status

from functools import wraps
from pathlib import Path

import environ
import json
import requests

BASE_DIR = Path(__file__).resolve().parent.parent

env = environ.Env()
env.read_env(BASE_DIR / '.env')

def check_API_key(view):
    @wraps(view)
    def wrapper(request):
        try:
            api_key = env('API_KEY')

            q = None
            if 'q' in request.data:
                try:
                    content = json.loads(request.data)
                    q = content.get('q')
                except Exception:
                    pass
            elif '_content' in request.data:
                try:
                    content = json.loads(request.data["_content"])
                    q = content.get('q')
                except Exception:
                    pass
            
            if q != api_key:
                return Response({'Error': 'API key is incorrect'}, status=status.HTTP_403_FORBIDDEN)
            
            request.content = content

            return view(request)
        except Exception:
            return Response({'Error': 'Invalid request'}, status=status.HTTP_400_BAD_REQUEST)

    return wrapper

@api_view(['POST'])
@check_API_key
def getAccessToken(request):
    code = request.content.get('code') # nie wiem jak sie nazywa ten parametr

    if code is not None:
        data = {
            'code': code,
            'client_id': env('GOOGLE_CLIENT_ID'),
            'client_secret': env('GOOGLE_CLIENT_SECRET'),
            'redirect_uri': 'http://localhost:8080',
            'grant_type': 'authorization_code'
        }

        tokens_response = requests.post('https://oauth2.googleapis.com/token', data=data)
        if tokens_response.status_code == 200:
            tokens = tokens_response.json()

            access_token = tokens["access_token"]
            refresh_token = tokens["refresh_token"]

            email_response = requests.get('https://openidconnect.googleapis.com/v1/userinfo', headers={'Authorization': f'Bearer {access_token}'})

            if email_response.status_code == 200:
                parsed_email_response = email_response.json()
                email = parsed_email_response["email"]

                response = {
                    'access_token': access_token,
                    'refresh_token': refresh_token,
                    'email': email
                }

                return Response(response, status=status.HTTP_200_OK)
            else:
                response = {
                    'access_token': access_token,
                    'refresh_token': refresh_token,
                    'email': 'not available'
                }

                return Response(response, status=status.HTTP_200_OK)
        else:
            return Response({'Error': 'Google did not send tokens back'}, status=status.HTTP_503_SERVICE_UNAVAILABLE)
    else:
        return Response({'Error': 'Google authentication code was not provided'}, status=status.HTTP_400_BAD_REQUEST)

@api_view(['POST'])
def upload(request):
    pass
from rest_framework.decorators import api_view
from rest_framework.response import Response
from rest_framework import status

from functools import wraps
from pathlib import Path

import environ
import json

BASE_DIR = Path(__file__).resolve().parent.parent.parent

def check_API_key(view):
    @wraps(view)
    def wrapper(request):
        env = environ.Env()
        env.read_env(BASE_DIR / '.env')

        api_key = env('API_KEY')

        q = None
        if 'q' in request.data:
            try:
                content = json.loads(request.data)
                q = content.get('q')
            except Exception:
                q = None
        
        if q != api_key:
            return Response({'Error': 'API key is incorrect'}, status=status.HTTP_403_FORBIDDEN)
        
        request.content = content

        return view(request)

    return wrapper

@api_view(['POST'])
def getAccessToken(request):
    pass

@api_view(['POST'])
def upload(request):
    pass
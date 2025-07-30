from django.http import HttpResponse
from django.template import loader

def main(request):
    status = request.GET.get('status')

    if status == 'ok':
        return HttpResponse(loader.get_template('main.html').render(request=request, context={'title': 'Success', 'message': 'You can close the browser now'}))
    else:
        reason = request.GET.get('reason')

        if reason == 'cb': # means callback wasn't set in frontend
            return HttpResponse(loader.get_template('main.html').render(request=request, context={'title': 'Error', 'message': 'Server error, please try again later :('})) # this shouldn't happen, if frontend's receiver is implemented correctly
        elif reason == 'parameters': # google didn't provided 'code' query parameter
            return HttpResponse(loader.get_template('main.html').render(request=request, context={'title': 'Error', 'message': 'Google stopped the verification, please try again'}))
        elif reason == 'type': # google send http request different than GET
            return HttpResponse(loader.get_template('main.html').render(request=request, context={'title': 'Error', 'message': 'Google discontinued the verificagion, please try again'}))
        elif reason == 'empty': # google's response was empty
            return HttpResponse(loader.get_template('main.html').render(request=request, context={'title': 'Error', 'message': 'Authentication code was not sent, please try again'}))
        else: # again, this only happens if fronted's listening socket is implemented badly
            return HttpResponse(loader.get_template('main.html').render(request=request, context={'title': 'Error', 'message': 'An error occured, please try again later :('}))

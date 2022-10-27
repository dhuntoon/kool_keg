import requests
import json
from sys import argv

kegId = argv[1]
temp = argv[2]
vol = argv[3]

data = {"kegId":str(kegId),"data":{"temp":str(temp),"vol":str(vol)}}
print(data)
r = requests.put('https://1cl4lag6ba.execute-api.us-east-2.amazonaws.com/prod/keglist',data=json.dumps(data),headers={'content-type':'application/json'})

print(r)

print(r.content)


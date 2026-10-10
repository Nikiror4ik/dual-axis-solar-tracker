from flask import Flask , request
import requests
app = Flask ( __name__ )
BOT_TOKEN = " Insert_Token here "
CHAT_ID = " Insert_ID here "
@app . route ("/ data ", methods =[" POST "])
def data ():
voltage = request . form .get (" voltage ")
light = request . form .get (" light ")
print (f" Received : voltage ={ voltage } V, light ={ light
}")
message = (
" Solar Tracker \n"
f" Voltage : { voltage } V\n"
f" Light : { light }%"
)
response = requests . post (
f" https :// api. telegram .org/bot{ BOT_TOKEN }/
sendMessage ",
data ={
" chat_id ": CHAT_ID ,
" text ": message
}
)
print (" Telegram response :", response . text )
return "OK"
app.run( host ="0.0.0.0" , port =8000)" , port =8000)
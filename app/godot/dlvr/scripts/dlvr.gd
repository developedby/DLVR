extends Node

const SERVER_URL := "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com:443/"

onready var ws:WebSocketClient = WebSocketClient.new()
var debug := true
var user_first_name:String = "DL"
var user_surname:String = "VR"

func _ready():
	ws.connect("connection_established", self, "_connection_established")
	ws.connect("connection_closed", self, "_connection_closed")
	ws.connect("connection_error", self, "_connection_error")

func set_first_name(val:String):
	user_first_name = val.strip_edges()

func set_surname(val:String):
	user_surname = val.strip_edges()

func logout():
	user_first_name = "DL"
	user_surname = "VR"

func _connection_established(protocol):
	if debug:
    	print("Connection established with protocol: ", protocol)

func _connection_closed():
	if debug:
    	print("Connection closed")

func _connection_error():
	if debug:
    	print("Connection error")

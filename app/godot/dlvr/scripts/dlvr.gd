extends Node

const SERVER_URL := "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com:443/"

var debug := true setget set_debug

var user_first_name:String = "DL"
var user_last_name:String = "VR"

func _ready():
	set_debug(true)

func set_first_name(val:String):
	user_first_name = val.strip_edges()

func set_last_name(val:String):
	user_last_name = val.strip_edges()

func logout():
	user_first_name = "DL"
	user_last_name = "VR"

func set_debug(val):
	debug = val
	Utils.debug = val

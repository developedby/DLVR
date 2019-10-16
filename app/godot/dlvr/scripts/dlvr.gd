extends Node

var user_first_name:String = "DL"
var user_surname:String = "VR"

func _ready():
	pass


func set_first_name(val:String):
	user_first_name = val.strip_edges()

func set_surname(val:String):
	user_surname = val.strip_edges()

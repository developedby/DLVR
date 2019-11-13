extends "res://scripts/SubView.gd"

signal code_entered(code, user_email)

var user_email:String = "dlvr@dlvr.com"


func _ready():
	pass


func initial():
	user_email = "dlvr@dlvr.com"


func set_data(email):
	user_email = email


func _on_button_c_pressed():
	emit_signal("code_entered", $code.text.strip_edges(), user_email)

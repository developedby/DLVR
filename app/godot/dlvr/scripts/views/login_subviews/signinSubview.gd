extends "res://scripts/SubView.gd"

signal data_entered(email, first_name, surname, passw)

onready var email_label := $emailLabel
onready var name_field := $first_name
onready var surname_field := $surname
onready var passw_field := $passw
onready var repassw_field := $re_passw

func _ready():
	# warning-ignore: unused_argument
	connect('data_entered', get_parent(), '_on_data_entered')


func initial():
	name_field.text = ""
	surname_field.text = ""
	passw_field.text = ""
	repassw_field.text = ""


func set_email(val):
	email_label.text = val


func _on_button_pressed():
	var first_name:String =  name_field.text.strip_edges()
	var surname:String = surname_field.text.strip_edges()
	var passw:String = passw_field.text.strip_edges()
	var repassw:String = repassw_field.text.strip_edges()
	if first_name.length() and surname.length() and $passw.valid and $re_passw.valid and (passw == repassw):
		emit_signal('data_entered', $emailLabel.text, first_name, surname, passw)

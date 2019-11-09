extends "res://scripts/SubView.gd"

# warning-ignore:unused_signal
signal email_entered(email)

onready var email_field = $emailField

func _ready():
	# warning-ignore: return_value_discarded
	connect("email_entered", get_parent(), "_on_email_entered")

func _on_button_pressed():
	if email_field.valid:
		emit_signal("email_entered", email_field.text.strip_edges())
	else:
		pass

func initial():
	email_field.text = ""
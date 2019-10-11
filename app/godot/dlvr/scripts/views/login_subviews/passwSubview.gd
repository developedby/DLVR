extends "res://scripts/SubView.gd"

# warning-ignore:unused_signal
signal passw_entered(email, passw)

onready var passw_field = $passwField
onready var email_label = $emailLabel

func _ready():
	# warning-ignore: return_value_discarded
	connect("passw_entered", get_parent(), "_on_passw_entered")

func _on_button_pressed():
	emit_signal("passw_entered",
				email_label.text.strip_edges(),
				passw_field.text.strip_edges())


func initial():
	email_label.text = ""
	passw_field.text = ""


func set_email(val):
	email_label.text = val

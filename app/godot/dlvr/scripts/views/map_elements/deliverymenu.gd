extends Control

onready var parent = get_parent()

func _ready():
	get_parent().connect("state_changed", self, "_on_state_changed")
	_on_state_changed(parent.current_state)

func _on_state_changed(val):
	if val != parent.STATE.ORIGIN_SET:
		$email.text = ""
	if val == parent.STATE.IDLE:
		$email.visible = false
		$cancel_button.visible = false
		$confirm_button.visible = false
	elif val == parent.STATE.ORIGIN_SET:
		$email.visible = true
		$cancel_button.visible = true
		$confirm_button.visible = true
	elif val == parent.STATE.SETTING_CURRENT_LOCATION:
		$email.visible = false
		$cancel_button.visible = false
		$confirm_button.visible = true

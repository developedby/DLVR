tool
extends Popup

class_name popupbox

export(String) var message = "" setget set_message

func _ready():
	pass

func _draw():
	$label.text = message

func set_message(val):
	message = val
	update()

func _on_ok_pressed():
	self.visible = false

func popup_m(val=null):
	if val != null:
		set_message(str(val))
	popup()

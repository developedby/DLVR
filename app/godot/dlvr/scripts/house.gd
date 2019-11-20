extends TouchScreenButton

class_name House

signal house_pressed(num)

export(int) var id:int = 0


func _ready():
	connect("released", self, "_on_button_released")

func _on_button_released():
	emit_signal("house_pressed", id)

func get_pos():
	return position
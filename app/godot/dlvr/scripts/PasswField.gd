extends LineEdit

class_name PasswField

const EMPTY_COLOR := Color(0.804, 0.804, 0.804)
const VALID_COLOR := Color(0.518, 0.714, 0.898)
const INVALID_COLOR := Color(0.898, 0.714, 0.518)


var valid := false
export(int) var min_size = -1


func _ready():
	pass


func set_valid():
	get_stylebox("focus").border_color = VALID_COLOR
	valid = true


func set_invalid():
	get_stylebox("focus").border_color = INVALID_COLOR
	valid = false


func set_empty():
	get_stylebox("focus").border_color = EMPTY_COLOR
	valid = false


func _draw():
	var value = self.text
	if value == "":
		set_empty()
	elif min_size <= value.length():
		set_valid()
	else:
		set_invalid()

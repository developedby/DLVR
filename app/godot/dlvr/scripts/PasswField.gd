extends LineEdit

class_name PasswField

const EMPTY_COLOR := Color(0.804, 0.804, 0.804)
const VALID_COLOR := Color(0.518, 0.714, 0.898)
const INVALID_COLOR := Color(0.898, 0.714, 0.518)

var equals_to:PasswField = null

var valid := false
export(int) var min_size = -1
export(NodePath) var passw_equals:NodePath = "" setget set_passw_equals


func _ready():
	set_passw_equals(passw_equals)


func set_valid():
	get_stylebox("focus").border_color = VALID_COLOR
	valid = true


func set_invalid():
	get_stylebox("focus").border_color = INVALID_COLOR
	valid = false


func set_empty():
	get_stylebox("focus").border_color = EMPTY_COLOR
	valid = false


func set_passw_equals(val):
	passw_equals = val
	equals_to = (get_node(val) as PasswField)


func _draw():
	var value = self.text
	var valid_map = true
	valid_map = valid_map and (min_size <= value.length())
	valid_map = valid_map and ((equals_to == null) or (equals_to.text == value))
	if value == "":
		set_empty()
	elif valid_map:
		set_valid()
	else:
		set_invalid()

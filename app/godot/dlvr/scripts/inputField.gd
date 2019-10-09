extends LineEdit


func _ready():
	pass


func set_border_color(col:Color):
	get_stylebox('focus').border_color = col

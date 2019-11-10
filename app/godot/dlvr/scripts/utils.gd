extends Node

var debug = true

func print_log(val):
    if debug:
        print(val)

func encode_data(data, mode):
	return data.to_utf8() if mode == WebSocketPeer.WRITE_MODE_TEXT else var2bytes(data)

func decode_data(data, is_string):
	return data.get_string_from_utf8() if is_string else bytes2var(data)

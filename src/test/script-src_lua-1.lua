print('Lua script started for src_lua')

function handle(msg)
	print('message source')
	local t = { msg_type = MSG_TIMER, data = { timer_id = 1234 }}
	return msg_from_table(msg, t)
end

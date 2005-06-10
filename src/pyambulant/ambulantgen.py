f = Method(void, 'initialize',
condition='#ifdef USE_SMIL21'
)
Player_methods.append(f)

f = Method(void, 'start'
)
Player_methods.append(f)

f = Method(void, 'stop'
)
Player_methods.append(f)

f = Method(void, 'pause'
)
Player_methods.append(f)

f = Method(void, 'resume'
)
Player_methods.append(f)

f = Method(bool, 'is_playing'
)
Player_methods.append(f)

f = Method(bool, 'is_pausing'
)
Player_methods.append(f)

f = Method(bool, 'is_done'
)
Player_methods.append(f)

f = Method(int, 'get_cursor'
)
Player_methods.append(f)

f = Method(void, 'set_cursor',
    (int, 'cursor', InMode)
)
Player_methods.append(f)


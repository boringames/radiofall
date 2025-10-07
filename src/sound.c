#include "sound.h"

static bool _sound_enabled = true;

void sound_set_enabled(bool enabled)
{
    _sound_enabled = enabled;
}

bool sound_enabled()
{
    return _sound_enabled;
}

void sound_play(Sound sound)
{
    if (_sound_enabled) PlaySound(sound);
}
package com.motovar.rocket;

import org.libsdl.app.SDLActivity;

/** The game itself: SDL's activity, which loads libSDL2.so and then libmain.so and runs its SDL_main(). */
public class RocketActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
    }
}

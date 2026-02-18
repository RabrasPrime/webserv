//
// Created by tjooris on 2/16/26.
//

#include "Engine.hpp"

Engine::Engine(): _is_running(false) {}

Engine::~Engine() {
	stop();
}

#pragma once


int enter_critical_context();

void exit_critical_context(int previous_state);

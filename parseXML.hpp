#include "implementDB.hpp"
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <string.h>

void handle_request(char *request, int size);
void handle_create(pugi::xml_document &doc, pugi::xml_document &response);

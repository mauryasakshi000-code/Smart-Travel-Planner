#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>

// Crow's App type is parameterised by its middleware list, so every
// translation unit that registers routes must agree on this single alias.
using CrowApp = crow::App<crow::CORSHandler>;

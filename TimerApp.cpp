#include <gtkmm.h>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    // 1. Create the application
    auto app = Gtk::Application::create(argc, argv, "com.yourname.nodistractions");

    // 2. Create the Main Window
    MainWindow window;

    // 3. Run the application
    return app->run(window);
}


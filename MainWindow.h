#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtkmm.h>
#include "WorkSession.h"
#include <vector>
#include <string>

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    virtual ~MainWindow();

protected:
    // Widgets
    Gtk::Box m_mainHBox;      // Main container (Horizontal: Image | Controls)
    Gtk::Box m_controlsVBox;  // Right side container (Vertical: Timer, Inputs, Buttons)
    Gtk::Box m_headerBox;     // Holds timer + spinner + status
    Gtk::Box m_timerRow;      // Timer label + spinner
    Gtk::Box m_buttonBox;
    Gtk::Frame m_imageFrame;
    Gtk::Box m_characterColumn;
    Gtk::Spinner m_spinner;
    Gtk::Label m_statusLabel;
    Gtk::Label m_timerLabel;
    Gtk::Entry m_nameEntry;
    Gtk::Entry m_descEntry;
    Gtk::Button m_startButton;
    Gtk::Button m_stopButton;
    Gtk::Button m_saveButton;
    Gtk::Button m_resetButton;
    Gtk::Button m_deleteButton;

    // Sessions panel
    Gtk::Frame m_sessionsFrame;
    Gtk::Box m_sessionsBox;
    Gtk::ScrolledWindow m_sessionsScroll;
    Gtk::ListBox m_sessionsList;
    Gtk::Box m_editBox;
    Gtk::Entry m_editName;
    Gtk::Entry m_editDesc;
    Gtk::Label m_editDuration;
    Gtk::Button m_updateButton;

    // Helpers
    void setup_css();
    void setup_ui();
    void load_characters();
    Glib::RefPtr<Gdk::Pixbuf> load_scaled_pixbuf(const std::string& path, int target_width);
    void update_running_state(bool running);
    void setup_sessions_panel();
    void refresh_sessions_list();
    void load_sessions_from_file();
    void write_sessions_to_file();
    std::string find_asset_path(const std::string& filename) const;
    void on_session_row_selected(Gtk::ListBoxRow* row);
    void on_update_session_clicked();
    void on_delete_session_clicked();
    void on_reset_clicked();

    // Signal handlers
    void on_start_clicked();
    void on_stop_clicked();
    void on_save_clicked();
    bool on_timeout();

    // Timer state
    bool m_isRunning;
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::seconds m_accumulatedTime;
    sigc::connection m_timeoutConnection;
    std::vector<Gtk::Image*> m_characterImages;
    std::vector<WorkSession> m_sessions;
};

#endif // MAINWINDOW_H

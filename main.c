#include <gtk/gtk.h>
#include <vte/vte.h>
#include <fontconfig/fontconfig.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *main_window = NULL;
static GtkWidget *root_box = NULL;

static void split_terminal(GtkWidget *widget, int direction, const char *cwd);
static void close_terminal(GtkWidget *term);
static GtkWidget* create_terminal_widget(const char *working_directory);
static void save_state();

// Data structures for themes
typedef struct {
    const char *name;
    GdkRGBA bg;
    GdkRGBA fg;
    GdkRGBA palette[16];
} TerminalTheme;

// Array of imaginative themes
static TerminalTheme themes[] = {
    {
        .name = "Cyberpunk Neon",
        .bg = {0.05, 0.05, 0.08, 1.0}, // Deep dark purple
        .fg = {0.0, 1.0, 0.8, 1.0},    // Neon Cyan
        .palette = {
            {0.1, 0.1, 0.1, 1.0}, {0.9, 0.1, 0.3, 1.0}, {0.1, 0.9, 0.4, 1.0}, {0.9, 0.9, 0.1, 1.0},
            {0.1, 0.4, 0.9, 1.0}, {0.9, 0.1, 0.9, 1.0}, {0.1, 0.9, 0.9, 1.0}, {0.8, 0.8, 0.8, 1.0},
            {0.4, 0.4, 0.4, 1.0}, {1.0, 0.2, 0.4, 1.0}, {0.2, 1.0, 0.5, 1.0}, {1.0, 1.0, 0.2, 1.0},
            {0.2, 0.5, 1.0, 1.0}, {1.0, 0.2, 1.0, 1.0}, {0.2, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}
        }
    },
    {
        .name = "Solar Flare",
        .bg = {0.95, 0.92, 0.85, 1.0}, // Warm cream
        .fg = {0.2, 0.15, 0.1, 1.0},   // Deep brown
        .palette = {
            {0.0, 0.0, 0.0, 1.0}, {0.8, 0.2, 0.1, 1.0}, {0.4, 0.6, 0.1, 1.0}, {0.8, 0.5, 0.0, 1.0},
            {0.1, 0.4, 0.7, 1.0}, {0.6, 0.2, 0.6, 1.0}, {0.1, 0.6, 0.6, 1.0}, {0.6, 0.6, 0.6, 1.0},
            {0.3, 0.3, 0.3, 1.0}, {0.9, 0.3, 0.2, 1.0}, {0.5, 0.7, 0.2, 1.0}, {0.9, 0.6, 0.1, 1.0},
            {0.2, 0.5, 0.8, 1.0}, {0.7, 0.3, 0.7, 1.0}, {0.2, 0.7, 0.7, 1.0}, {0.9, 0.9, 0.9, 1.0}
        }
    },
    {
        .name = "Deep Ocean",
        .bg = {0.02, 0.1, 0.15, 1.0},  // Abyss blue
        .fg = {0.8, 0.9, 0.95, 1.0},   // Sea foam
        .palette = {
            {0.05, 0.1, 0.15, 1.0}, {0.8, 0.3, 0.3, 1.0}, {0.2, 0.8, 0.5, 1.0}, {0.8, 0.8, 0.2, 1.0},
            {0.2, 0.4, 0.8, 1.0}, {0.8, 0.3, 0.8, 1.0}, {0.2, 0.8, 0.8, 1.0}, {0.7, 0.8, 0.9, 1.0},
            {0.2, 0.3, 0.4, 1.0}, {1.0, 0.4, 0.4, 1.0}, {0.3, 0.9, 0.6, 1.0}, {0.9, 0.9, 0.3, 1.0},
            {0.3, 0.5, 0.9, 1.0}, {0.9, 0.4, 0.9, 1.0}, {0.3, 0.9, 0.9, 1.0}, {0.9, 0.95, 1.0, 1.0}
        }
    },
    {
        .name = "Hacker Green",
        .bg = {0.0, 0.0, 0.0, 1.0},    // Pure black
        .fg = {0.1, 0.9, 0.1, 1.0},    // Retro phosphor green
        .palette = {
            {0.0, 0.0, 0.0, 1.0}, {0.7, 0.0, 0.0, 1.0}, {0.0, 0.7, 0.0, 1.0}, {0.7, 0.7, 0.0, 1.0},
            {0.0, 0.0, 0.7, 1.0}, {0.7, 0.0, 0.7, 1.0}, {0.0, 0.7, 0.7, 1.0}, {0.7, 0.7, 0.7, 1.0},
            {0.3, 0.3, 0.3, 1.0}, {1.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {1.0, 1.0, 0.0, 1.0},
            {0.0, 0.0, 1.0, 1.0}, {1.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}
        }
    },
    {
        .name = "Default Dark",
        .bg = {0.117, 0.117, 0.117, 1.0}, 
        .fg = {0.9, 0.9, 0.9, 1.0},       
        .palette = {
            {0.0, 0.0, 0.0, 1.0}, {0.8, 0.0, 0.0, 1.0}, {0.0, 0.8, 0.0, 1.0}, {0.8, 0.8, 0.0, 1.0},
            {0.0, 0.0, 0.8, 1.0}, {0.8, 0.0, 0.8, 1.0}, {0.0, 0.8, 0.8, 1.0}, {0.8, 0.8, 0.8, 1.0},
            {0.5, 0.5, 0.5, 1.0}, {1.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {1.0, 1.0, 0.0, 1.0},
            {0.0, 0.0, 1.0, 1.0}, {1.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}
        }
    }
};

static int current_theme_index = 4; // Default Dark

static void apply_theme_to_terminal(GtkWidget *widget, gpointer user_data) {
    if (VTE_IS_TERMINAL(widget)) {
        TerminalTheme *t = (TerminalTheme*)user_data;
        vte_terminal_set_colors(VTE_TERMINAL(widget), &t->fg, &t->bg, t->palette, 16);
    } else if (GTK_IS_CONTAINER(widget)) {
        gtk_container_foreach(GTK_CONTAINER(widget), apply_theme_to_terminal, user_data);
    }
}

static void on_theme_selected(GtkMenuItem *item, gpointer user_data) {
    int index = GPOINTER_TO_INT(user_data);
    current_theme_index = index;
    apply_theme_to_terminal(root_box, &themes[index]);
}

static void show_theme_menu(GtkWidget *widget, gpointer user_data) {
    GtkWidget *menu = gtk_menu_new();
    
    int num_themes = sizeof(themes) / sizeof(themes[0]);
    for (int i = 0; i < num_themes; i++) {
        GtkWidget *item = gtk_menu_item_new_with_label(themes[i].name);
        g_signal_connect(item, "activate", G_CALLBACK(on_theme_selected), GINT_TO_POINTER(i));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }
    
    gtk_widget_show_all(menu);
    gtk_menu_popup_at_widget(GTK_MENU(menu), widget, GDK_GRAVITY_SOUTH_EAST, GDK_GRAVITY_NORTH_EAST, NULL);
}

static void apply_custom_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const gchar *css =
        "paned > separator {"
        "   background-color: #333333;"
        "   min-width: 6px;"
        "   min-height: 6px;"
        "}"
        "paned > separator:hover {"
        "   background-color: #007acc;"
        "}";
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void load_custom_font() {
    // Attempt to load the font from assets/fonts/UbuntuMono-Regular.ttf
    FcConfig *config = FcInitLoadConfigAndFonts();
    FcBool font_added = FcConfigAppFontAddFile(config, (const FcChar8 *)"assets/fonts/UbuntuMono-Regular.ttf");
    if (font_added) {
        FcConfigSetCurrent(config);
    } else {
        // Fallback to system fonts, not ideal but safe
    }
}

static void on_term_child_exited(VteTerminal *term, gint status, gpointer user_data) {
    close_terminal(GTK_WIDGET(term));
}

static void on_copy(GtkMenuItem *item, gpointer user_data) {
    VteTerminal *term = VTE_TERMINAL(user_data);
    vte_terminal_copy_clipboard_format(term, VTE_FORMAT_TEXT);
}

static void on_paste(GtkMenuItem *item, gpointer user_data) {
    VteTerminal *term = VTE_TERMINAL(user_data);
    vte_terminal_paste_clipboard(term);
}

static void on_split_h(GtkMenuItem *item, gpointer user_data) {
    VteTerminal *term = VTE_TERMINAL(user_data);
    const char *uri = vte_terminal_get_current_directory_uri(term);
    char *cwd = NULL;
    if (uri) cwd = g_filename_from_uri(uri, NULL, NULL);
    split_terminal(GTK_WIDGET(term), 0, cwd);
    g_free(cwd);
}

static void on_split_v(GtkMenuItem *item, gpointer user_data) {
    VteTerminal *term = VTE_TERMINAL(user_data);
    const char *uri = vte_terminal_get_current_directory_uri(term);
    char *cwd = NULL;
    if (uri) cwd = g_filename_from_uri(uri, NULL, NULL);
    split_terminal(GTK_WIDGET(term), 1, cwd);
    g_free(cwd);
}

static void on_close_action(GtkMenuItem *item, gpointer user_data) {
    close_terminal(GTK_WIDGET(user_data));
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        GtkWidget *menu = gtk_menu_new();

        GtkWidget *item_copy = gtk_menu_item_new_with_label("Copy");
        g_signal_connect(item_copy, "activate", G_CALLBACK(on_copy), widget);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_copy);

        GtkWidget *item_paste = gtk_menu_item_new_with_label("Paste");
        g_signal_connect(item_paste, "activate", G_CALLBACK(on_paste), widget);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_paste);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

        GtkWidget *item_split_h = gtk_menu_item_new_with_label("Split Horizontally");
        g_signal_connect(item_split_h, "activate", G_CALLBACK(on_split_h), widget);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_split_h);

        GtkWidget *item_split_v = gtk_menu_item_new_with_label("Split Vertically");
        g_signal_connect(item_split_v, "activate", G_CALLBACK(on_split_v), widget);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_split_v);

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

        GtkWidget *item_close = gtk_menu_item_new_with_label("Close Terminal Panel");
        g_signal_connect(item_close, "activate", G_CALLBACK(on_close_action), widget);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_close);

        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)event);
        return TRUE;
    }
    return FALSE;
}

static GtkWidget* create_terminal_widget(const char *working_directory) {
    GtkWidget *term = vte_terminal_new();
    
    // Set font
    PangoFontDescription *font_desc = pango_font_description_from_string("UbuntuMono 12");
    vte_terminal_set_font(VTE_TERMINAL(term), font_desc);
    pango_font_description_free(font_desc);

    // Setup colors from current theme
    TerminalTheme *t = &themes[current_theme_index];
    vte_terminal_set_colors(VTE_TERMINAL(term), &t->fg, &t->bg, t->palette, 16);
    
    // Setup mouse events
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(term), TRUE);
    g_signal_connect(term, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(term, "child-exited", G_CALLBACK(on_term_child_exited), NULL);

    // Spawn shell
    char *shell = getenv("SHELL");
    if (!shell) shell = "/bin/bash";
    char **command = g_new(char *, 2);
    command[0] = g_strdup(shell);
    command[1] = NULL;

    const char *cwd = (working_directory && g_file_test(working_directory, G_FILE_TEST_IS_DIR)) ? working_directory : g_get_home_dir();

    vte_terminal_spawn_async(
        VTE_TERMINAL(term),
        VTE_PTY_DEFAULT,
        cwd,
        command,
        NULL,
        G_SPAWN_DEFAULT,
        NULL, NULL,
        NULL,
        -1,
        NULL, NULL, NULL
    );

    g_strfreev(command);
    return term;
}

static void split_terminal(GtkWidget *widget, int direction, const char *cwd) {
    GtkWidget *parent = gtk_widget_get_parent(widget);
    GtkWidget *new_term = create_terminal_widget(cwd);
    
    // direction 0 = horiz (side by side), 1 = vert (top and bottom)
    GtkWidget *paned = direction == 0 ? gtk_paned_new(GTK_ORIENTATION_HORIZONTAL) : gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_wide_handle(GTK_PANED(paned), TRUE);
    
    // To retain the sizing nicely
    g_object_ref(widget);
    
    if (GTK_IS_CONTAINER(parent)) {
        gtk_container_remove(GTK_CONTAINER(parent), widget);
    }
    
    gtk_paned_pack1(GTK_PANED(paned), widget, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(paned), new_term, TRUE, FALSE);
    
    if (GTK_IS_CONTAINER(parent)) {
        gtk_container_add(GTK_CONTAINER(parent), paned);
    }
    
    g_object_unref(widget);
    gtk_widget_show_all(parent);
    gtk_widget_grab_focus(new_term);
}

static void close_terminal(GtkWidget *term) {
    GtkWidget *parent = gtk_widget_get_parent(term);
    
    if (parent == root_box) {
        // Last terminal, close app
        save_state();
        gtk_widget_destroy(main_window);
        return;
    }
    
    if (GTK_IS_PANED(parent)) {
        GtkWidget *other_child = (gtk_paned_get_child1(GTK_PANED(parent)) == term) ? 
                                 gtk_paned_get_child2(GTK_PANED(parent)) : 
                                 gtk_paned_get_child1(GTK_PANED(parent));
        
        GtkWidget *grandparent = gtk_widget_get_parent(parent);
        
        if (other_child) {
            g_object_ref(other_child);
            gtk_container_remove(GTK_CONTAINER(parent), other_child);
            
            if (GTK_IS_PANED(grandparent)) {
                // If the parent was packed as child1 or child2
                if (gtk_paned_get_child1(GTK_PANED(grandparent)) == parent) {
                    gtk_container_remove(GTK_CONTAINER(grandparent), parent);
                    gtk_paned_pack1(GTK_PANED(grandparent), other_child, TRUE, FALSE);
                } else {
                    gtk_container_remove(GTK_CONTAINER(grandparent), parent);
                    gtk_paned_pack2(GTK_PANED(grandparent), other_child, TRUE, FALSE);
                }
            } else if (GTK_IS_CONTAINER(grandparent)) {
                gtk_container_remove(GTK_CONTAINER(grandparent), parent);
                gtk_container_add(GTK_CONTAINER(grandparent), other_child);
            }
            
            g_object_unref(other_child);
        } else {
            gtk_widget_destroy(parent); // Failsafe
        }
    }
}

static void write_widget_state(GtkWidget *widget, FILE *f) {
    if (VTE_IS_TERMINAL(widget)) {
        const char *uri = vte_terminal_get_current_directory_uri(VTE_TERMINAL(widget));
        char *cwd = NULL;
        if (uri) cwd = g_filename_from_uri(uri, NULL, NULL);
        fprintf(f, "TERM %s\n", cwd ? cwd : "");
        g_free(cwd);
    } else if (GTK_IS_PANED(widget)) {
        GtkOrientation orient = gtk_orientable_get_orientation(GTK_ORIENTABLE(widget));
        fprintf(f, "PANED %d\n", orient);
        GtkWidget *child1 = gtk_paned_get_child1(GTK_PANED(widget));
        GtkWidget *child2 = gtk_paned_get_child2(GTK_PANED(widget));
        if (child1) write_widget_state(child1, f);
        if (child2) write_widget_state(child2, f);
    }
}

static GtkWidget* read_widget_state(FILE *f) {
    char line[1024];
    if (!fgets(line, sizeof(line), f)) return NULL;

    if (strncmp(line, "TERM", 4) == 0) {
        char cwd[1024] = {0};
        if (strlen(line) > 5) {
            strcpy(cwd, line + 5);
            cwd[strcspn(cwd, "\n")] = 0;
        }
        return create_terminal_widget(cwd[0] ? cwd : NULL);
    } else if (strncmp(line, "PANED", 5) == 0) {
        int orient = atoi(line + 6);
        GtkWidget *paned = gtk_paned_new(orient);
        gtk_paned_set_wide_handle(GTK_PANED(paned), TRUE);
        
        GtkWidget *child1 = read_widget_state(f);
        GtkWidget *child2 = read_widget_state(f);
        
        if (child1) gtk_paned_pack1(GTK_PANED(paned), child1, TRUE, FALSE);
        if (child2) gtk_paned_pack2(GTK_PANED(paned), child2, TRUE, FALSE);
        return paned;
    }
    return NULL;
}

static void save_state() {
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/cli", g_get_home_dir());
    g_mkdir_with_parents(path, 0755);
    snprintf(path, sizeof(path), "%s/.config/cli/state.txt", g_get_home_dir());
    FILE *f = fopen(path, "w");
    if (!f) return;
    
    fprintf(f, "THEME %d\n", current_theme_index);
    
    GList *children = gtk_container_get_children(GTK_CONTAINER(root_box));
    if (children && children->data) {
        write_widget_state(GTK_WIDGET(children->data), f);
    }
    g_list_free(children);
    fclose(f);
}

static void load_state() {
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/cli/state.txt", g_get_home_dir());
    FILE *f = fopen(path, "r");
    if (!f) {
        GtkWidget *initial_term = create_terminal_widget(NULL);
        gtk_box_pack_start(GTK_BOX(root_box), initial_term, TRUE, TRUE, 0);
        return;
    }

    char line[1024];
    if (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "THEME", 5) == 0) {
            current_theme_index = atoi(line + 6);
            int num_themes = sizeof(themes) / sizeof(themes[0]);
            if (current_theme_index < 0 || current_theme_index >= num_themes) {
                current_theme_index = 4;
            }
        }
    }

    GtkWidget *root_widget = read_widget_state(f);
    fclose(f);

    if (root_widget) {
        gtk_box_pack_start(GTK_BOX(root_box), root_widget, TRUE, TRUE, 0);
    } else {
        GtkWidget *initial_term = create_terminal_widget(NULL);
        gtk_box_pack_start(GTK_BOX(root_box), initial_term, TRUE, TRUE, 0);
    }
}

static void on_app_quit(GtkWidget *widget, gpointer user_data) {
    save_state();
    gtk_main_quit();
}

static void grab_first_terminal_focus(GtkWidget *widget, gpointer user_data) {
    gboolean *found = (gboolean *)user_data;
    if (*found) return;
    
    if (VTE_IS_TERMINAL(widget)) {
        gtk_widget_grab_focus(widget);
        *found = TRUE;
    } else if (GTK_IS_CONTAINER(widget)) {
        gtk_container_foreach(GTK_CONTAINER(widget), grab_first_terminal_focus, user_data);
    }
}

#ifndef TEST_MODE
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    apply_custom_css();
    load_custom_font();

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1024, 768);
    gtk_window_set_title(GTK_WINDOW(main_window), "Cli");
    
    // Set custom icon
    GError *error = NULL;
    GdkPixbuf *icon = gdk_pixbuf_new_from_file("assets/icon.png", &error);
    if (icon) {
        gtk_window_set_icon(GTK_WINDOW(main_window), icon);
    } else {
        g_printerr("Error loading icon: %s\n", error->message);
        g_clear_error(&error);
    }

    // Custom HeaderBar (Title bar)
    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Cli");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "GPU Terminal");
    
    // Add icon to header bar
    if (icon) {
        GdkPixbuf *scaled_icon = gdk_pixbuf_scale_simple(icon, 24, 24, GDK_INTERP_BILINEAR);
        if (scaled_icon) {
            GtkWidget *icon_img = gtk_image_new_from_pixbuf(scaled_icon);
            gtk_header_bar_pack_start(GTK_HEADER_BAR(header), icon_img);
            g_object_unref(scaled_icon);
        }
        g_object_unref(icon);
    }
    
    // Theme button
    GtkWidget *theme_btn = gtk_button_new_with_label("Themes 🎨");
    gtk_widget_set_tooltip_text(theme_btn, "Change Terminal Theme");
    g_signal_connect(theme_btn, "clicked", G_CALLBACK(show_theme_menu), NULL);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), theme_btn);
    
    gtk_window_set_titlebar(GTK_WINDOW(main_window), header);

    root_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), root_box);

    load_state();

    g_signal_connect(main_window, "destroy", G_CALLBACK(on_app_quit), NULL);

    gtk_widget_show_all(main_window);
    
    gboolean found = FALSE;
    grab_first_terminal_focus(root_box, &found);

    gtk_main();

    return 0;
}
#endif

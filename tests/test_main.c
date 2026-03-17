#include <stdio.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

// Mock functions to avoid segfaults and aborts when GTK loop isn't running
void mock_copy(VteTerminal *term, VteFormat format) {}
void mock_paste(VteTerminal *term) {}
void mock_gtk_main_quit(void) {}
void mock_gtk_menu_popup_at_pointer(GtkMenu *menu, const GdkEvent *trigger_event) {}
void mock_gtk_menu_popup_at_widget(GtkMenu *menu, GtkWidget *widget, GdkGravity widget_anchor, GdkGravity menu_anchor, const GdkEvent *trigger_event) {}

#define vte_terminal_copy_clipboard_format mock_copy
#define vte_terminal_paste_clipboard mock_paste
#define gtk_main_quit mock_gtk_main_quit
#define gtk_menu_popup_at_pointer mock_gtk_menu_popup_at_pointer
#define gtk_menu_popup_at_widget mock_gtk_menu_popup_at_widget

// Include the entire main file so we can test its static functions
#include "../main.c"

// Mocks & Setup
static void setup() {
    int argc = 1;
    char *argv_data[] = {"cli_test", NULL};
    char **argv = argv_data;
    gtk_init(&argc, &argv);

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    root_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), root_box);
}

static void teardown() {
    if (main_window) {
        gtk_widget_destroy(main_window);
        main_window = NULL;
        root_box = NULL;
    }
}

// Tests
static void test_create_terminal_widget() {
    setup();
    GtkWidget *panel = create_terminal_widget(NULL, current_theme_index);
    assert(GTK_IS_OVERLAY(panel));
    assert(VTE_IS_TERMINAL(get_vte_from_panel(panel)));
    teardown();
}

static void test_split_terminal_horizontal() {
    setup();
    GtkWidget *panel1 = create_terminal_widget(NULL, current_theme_index);
    gtk_box_pack_start(GTK_BOX(root_box), panel1, TRUE, TRUE, 0);
    
    // Split horizontally
    split_terminal(panel1, 0, NULL);
    
    // Root box should now contain a paned instead of the raw terminal
    GList *children = gtk_container_get_children(GTK_CONTAINER(root_box));
    assert(children != NULL);
    GtkWidget *paned = GTK_WIDGET(children->data);
    assert(GTK_IS_PANED(paned));
    assert(gtk_orientable_get_orientation(GTK_ORIENTABLE(paned)) == GTK_ORIENTATION_HORIZONTAL);
    
    g_list_free(children);
    teardown();
}

static void test_split_terminal_vertical() {
    setup();
    GtkWidget *panel1 = create_terminal_widget(NULL, current_theme_index);
    gtk_box_pack_start(GTK_BOX(root_box), panel1, TRUE, TRUE, 0);
    
    // Split vertically
    split_terminal(panel1, 1, NULL);
    
    GList *children = gtk_container_get_children(GTK_CONTAINER(root_box));
    assert(children != NULL);
    GtkWidget *paned = GTK_WIDGET(children->data);
    assert(GTK_IS_PANED(paned));
    assert(gtk_orientable_get_orientation(GTK_ORIENTABLE(paned)) == GTK_ORIENTATION_VERTICAL);
    
    g_list_free(children);
    teardown();
}

static void test_close_terminal_nested() {
    setup();
    GtkWidget *panel1 = create_terminal_widget(NULL, current_theme_index);
    gtk_box_pack_start(GTK_BOX(root_box), panel1, TRUE, TRUE, 0);
    
    split_terminal(panel1, 0, NULL);
    
    GList *children = gtk_container_get_children(GTK_CONTAINER(root_box));
    GtkWidget *paned = GTK_WIDGET(children->data);
    
    GtkWidget *p1 = gtk_paned_get_child1(GTK_PANED(paned));
    GtkWidget *p2 = gtk_paned_get_child2(GTK_PANED(paned));
    assert(GTK_IS_OVERLAY(p1));
    assert(GTK_IS_OVERLAY(p2));
    
    close_terminal(p1);
    g_list_free(children);
    
    children = gtk_container_get_children(GTK_CONTAINER(root_box));
    GtkWidget *new_root_child = GTK_WIDGET(children->data);
    assert(GTK_IS_OVERLAY(new_root_child));
    g_list_free(children);

    split_terminal(new_root_child, 1, NULL);
    children = gtk_container_get_children(GTK_CONTAINER(root_box));
    GtkWidget *paned1 = GTK_WIDGET(children->data);
    GtkWidget *p3 = gtk_paned_get_child1(GTK_PANED(paned1));
    split_terminal(p3, 0, NULL);
    
    GtkWidget *paned2 = gtk_paned_get_child1(GTK_PANED(paned1));
    GtkWidget *p4 = gtk_paned_get_child1(GTK_PANED(paned2));
    close_terminal(p4);
    g_list_free(children);

    children = gtk_container_get_children(GTK_CONTAINER(root_box));
    paned1 = GTK_WIDGET(children->data);
    if (GTK_IS_PANED(paned1)) {
        GtkWidget *p7 = create_terminal_widget(NULL, current_theme_index);
        gtk_paned_pack2(GTK_PANED(paned1), p7, TRUE, TRUE); 
        split_terminal(p7, 0, NULL);
        GtkWidget *paned3 = gtk_paned_get_child2(GTK_PANED(paned1));
        if (GTK_IS_PANED(paned3)) {
            GtkWidget *p8 = gtk_paned_get_child1(GTK_PANED(paned3));
            GtkWidget *p9 = gtk_paned_get_child2(GTK_PANED(paned3));
            close_terminal(p9);
            
            split_terminal(p8, 0, NULL);
            GtkWidget *paned4 = gtk_paned_get_child2(GTK_PANED(paned1)); 
            if (GTK_IS_PANED(paned4)) {
                GtkWidget *p10 = gtk_paned_get_child1(GTK_PANED(paned4));
                close_terminal(p10); 
            }
        }
    }
    g_list_free(children);

    GtkWidget *paned_failsafe = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *p_failsafe = create_terminal_widget(NULL, current_theme_index);
    gtk_paned_pack1(GTK_PANED(paned_failsafe), p_failsafe, TRUE, TRUE);
    gtk_box_pack_start(GTK_BOX(root_box), paned_failsafe, TRUE, TRUE, 0);
    close_terminal(p_failsafe);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *p6 = create_terminal_widget(NULL, current_theme_index);
    gtk_container_add(GTK_CONTAINER(box), p6);
    gtk_box_pack_start(GTK_BOX(root_box), box, TRUE, TRUE, 0);
    close_terminal(p6);

    children = gtk_container_get_children(GTK_CONTAINER(root_box));
    if (children) {
        GList *it = children;
        while (it) {
            GtkWidget *w = GTK_WIDGET(it->data);
            if (GTK_IS_PANED(w)) {
                close_terminal(gtk_paned_get_child1(GTK_PANED(w)));
            } else {
                close_terminal(w);
            }
            it = it->next;
        }
        g_list_free(children);
    }
    
    main_window = NULL;
    root_box = NULL;
    teardown();
}

static void test_panel_theme_application() {
    setup();
    GtkWidget *panel = create_terminal_widget(NULL, 0);
    gtk_box_pack_start(GTK_BOX(root_box), panel, TRUE, TRUE, 0);
    
    GtkWidget *item = gtk_menu_item_new_with_label("Hacker Green");
    g_object_set_data(G_OBJECT(item), "theme-index", GINT_TO_POINTER(3));
    
    on_panel_theme_selected(GTK_MENU_ITEM(item), panel);
    
    int theme_idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(panel), "theme-index"));
    assert(theme_idx == 3);
    
    show_panel_theme_menu(NULL, panel);
    
    teardown();
}

static void test_ui_callbacks() {
    setup();
    GtkWidget *panel1 = create_terminal_widget(NULL, current_theme_index);
    gtk_box_pack_start(GTK_BOX(root_box), panel1, TRUE, TRUE, 0);
    GtkWidget *vte = get_vte_from_panel(panel1);
    
    // Execute callbacks directly
    on_split_h(NULL, panel1);
    on_split_v(NULL, panel1);

    on_copy(NULL, panel1);
    on_paste(NULL, panel1);
    
    on_term_child_exited(VTE_TERMINAL(vte), 0, NULL);

    teardown();
}

static void test_button_press() {
    setup();
    GtkWidget *panel1 = create_terminal_widget(NULL, current_theme_index);
    gtk_box_pack_start(GTK_BOX(root_box), panel1, TRUE, TRUE, 0);
    GtkWidget *vte = get_vte_from_panel(panel1);

    GdkEventButton event;
    event.type = GDK_BUTTON_PRESS;
    event.button = 3;
    on_button_press(vte, &event, NULL);
    
    event.button = 1;
    on_button_press(vte, &event, NULL);

    teardown();
}

static void test_state_persistence() {
    setup();
    
    GtkWidget *panel1 = create_terminal_widget(NULL, 1);
    gtk_box_pack_start(GTK_BOX(root_box), panel1, TRUE, TRUE, 0);
    split_terminal(panel1, 0, NULL); 
    
    save_state();
    
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/cli/state.txt", g_get_home_dir());
    FILE *f = fopen(path, "r");
    assert(f != NULL);
    
    char line[1024];
    fgets(line, sizeof(line), f); // THEME line
    fgets(line, sizeof(line), f); // PANED line
    fgets(line, sizeof(line), f); // TERM line
    assert(strncmp(line, "TERM 1", 6) == 0);
    fclose(f);

    teardown();
    setup();
    load_state();
    
    teardown();
}

static void test_initialization() {
    setup();
    apply_custom_css();
    load_custom_font();
    
    GtkWidget *panel1 = create_terminal_widget(NULL, current_theme_index);
    gtk_box_pack_start(GTK_BOX(root_box), panel1, TRUE, TRUE, 0);
    
    gboolean found = FALSE;
    grab_first_terminal_focus(root_box, &found);
    assert(found == TRUE);

    on_close_action(NULL, panel1);
    on_app_quit(NULL, NULL);
    
    teardown();
}

int main() {
    printf("Running test_create_terminal_widget...\n");
    test_create_terminal_widget();
    
    printf("Running test_split_terminal_horizontal...\n");
    test_split_terminal_horizontal();
    
    printf("Running test_split_terminal_vertical...\n");
    test_split_terminal_vertical();
    
    printf("Running test_close_terminal_nested...\n");
    test_close_terminal_nested();
    
    printf("Running test_panel_theme_application...\n");
    test_panel_theme_application();

    printf("Running test_ui_callbacks...\n");
    test_ui_callbacks();
    
    printf("Running test_button_press...\n");
    test_button_press();

    printf("Running test_state_persistence...\n");
    test_state_persistence();

    printf("Running test_initialization...\n");
    test_initialization();

    printf("\nAll tests passed successfully!\n");
    return 0;
}

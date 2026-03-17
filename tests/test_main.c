#include <stdio.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

// Mock functions to avoid segfaults and aborts when GTK loop isn't running
void mock_copy(VteTerminal *term, VteFormat format) {}
void mock_paste(VteTerminal *term) {}
void mock_gtk_main_quit(void) {}
void mock_gtk_menu_popup_at_pointer(GtkMenu *menu, const GdkEvent *trigger_event) {}

#define vte_terminal_copy_clipboard_format mock_copy
#define vte_terminal_paste_clipboard mock_paste
#define gtk_main_quit mock_gtk_main_quit
#define gtk_menu_popup_at_pointer mock_gtk_menu_popup_at_pointer

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
    GtkWidget *term = create_terminal_widget(NULL);
    assert(VTE_IS_TERMINAL(term));
    teardown();
}

static void test_split_terminal_horizontal() {
    setup();
    GtkWidget *term1 = create_terminal_widget(NULL);
    gtk_box_pack_start(GTK_BOX(root_box), term1, TRUE, TRUE, 0);
    
    // Split horizontally
    split_terminal(term1, 0, NULL);
    
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
    GtkWidget *term1 = create_terminal_widget(NULL);
    gtk_box_pack_start(GTK_BOX(root_box), term1, TRUE, TRUE, 0);
    
    // Split vertically
    split_terminal(term1, 1, NULL);
    
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
    GtkWidget *term1 = create_terminal_widget(NULL);
    gtk_box_pack_start(GTK_BOX(root_box), term1, TRUE, TRUE, 0);
    
    // Create a split
    split_terminal(term1, 0, NULL);
    
    GList *children = gtk_container_get_children(GTK_CONTAINER(root_box));
    GtkWidget *paned = GTK_WIDGET(children->data);
    
    GtkWidget *t1 = gtk_paned_get_child1(GTK_PANED(paned));
    GtkWidget *t2 = gtk_paned_get_child2(GTK_PANED(paned));
    assert(VTE_IS_TERMINAL(t1));
    assert(VTE_IS_TERMINAL(t2));
    
    // Close t1, paned should collapse and t2 should move to root
    close_terminal(t1);
    
    g_list_free(children);
    
    children = gtk_container_get_children(GTK_CONTAINER(root_box));
    GtkWidget *new_root_child = GTK_WIDGET(children->data);
    assert(VTE_IS_TERMINAL(new_root_child));
    
    g_list_free(children);

    // Deep nested test
    // root -> paned1 -> paned2 -> t3/t4
    //                 -> t5
    split_terminal(new_root_child, 1, NULL);
    children = gtk_container_get_children(GTK_CONTAINER(root_box));
    GtkWidget *paned1 = GTK_WIDGET(children->data);
    GtkWidget *t3 = gtk_paned_get_child1(GTK_PANED(paned1));
    split_terminal(t3, 0, NULL);
    
    GtkWidget *paned2 = gtk_paned_get_child1(GTK_PANED(paned1));
    GtkWidget *t4 = gtk_paned_get_child1(GTK_PANED(paned2));
    
    // Close t4 should collapse paned2 into paned1 child1
    close_terminal(t4);

    // To hit grandparent child2 block:
    // Split new_root_child (which might be child2 of paned1)
    GtkWidget *t7 = create_terminal_widget(NULL);
    gtk_paned_pack2(GTK_PANED(paned1), t7, TRUE, TRUE); // Ensure t7 is child2
    split_terminal(t7, 0, NULL);
    GtkWidget *paned3 = gtk_paned_get_child2(GTK_PANED(paned1));
    GtkWidget *t8 = gtk_paned_get_child1(GTK_PANED(paned3));
    GtkWidget *t9 = gtk_paned_get_child2(GTK_PANED(paned3));
    // Close child2 (t9) to hit the ternary else branch
    close_terminal(t9);
    // Now paned3 collapsed. Let's do it again to close a parent that is child2 of grandparent
    split_terminal(t8, 0, NULL);
    GtkWidget *paned4 = gtk_paned_get_child2(GTK_PANED(paned1)); // the new paned
    GtkWidget *t10 = gtk_paned_get_child1(GTK_PANED(paned4));
    close_terminal(t10); // this hits: grandparent is paned1, parent is paned4 (child2), so pack2 is called!

    // Failsafe block: paned with no other child
    GtkWidget *paned_failsafe = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *t_failsafe = create_terminal_widget(NULL);
    gtk_paned_pack1(GTK_PANED(paned_failsafe), t_failsafe, TRUE, TRUE);
    gtk_box_pack_start(GTK_BOX(root_box), paned_failsafe, TRUE, TRUE, 0);
    close_terminal(t_failsafe);

    // To hit grandparent container block, put a terminal inside an event box or something
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *t6 = create_terminal_widget(NULL);
    gtk_container_add(GTK_CONTAINER(box), t6);
    gtk_box_pack_start(GTK_BOX(root_box), box, TRUE, TRUE, 0);
    close_terminal(t6);

    // Also close the last terminal in root_box
    g_list_free(children);
    children = gtk_container_get_children(GTK_CONTAINER(root_box));
    if (children && children->data) {
        GtkWidget *last_paned = GTK_WIDGET(children->data);
        if (GTK_IS_PANED(last_paned)) {
            close_terminal(gtk_paned_get_child1(GTK_PANED(last_paned)));
        }
    }
    g_list_free(children);

    children = gtk_container_get_children(GTK_CONTAINER(root_box));
    if (children && children->data) {
        close_terminal(GTK_WIDGET(children->data));
    }
    g_list_free(children);
    
    // main_window should now be destroyed by close_terminal
    // we set it to NULL so teardown doesn't double-free
    main_window = NULL;
    root_box = NULL;
    
    teardown();
}

static void test_theme_application() {
    setup();
    GtkWidget *term1 = create_terminal_widget(NULL);
    gtk_box_pack_start(GTK_BOX(root_box), term1, TRUE, TRUE, 0);
    
    // Apply "Hacker Green" (index 3)
    current_theme_index = 3;
    apply_theme_to_terminal(root_box, &themes[current_theme_index]);
    
    assert(current_theme_index == 3);
    
    // Call UI handler directly
    on_theme_selected(NULL, GINT_TO_POINTER(1));
    assert(current_theme_index == 1);

    // Call show theme menu (it won't actually block since gtk_main isn't running)
    show_theme_menu(term1, NULL);
    
    teardown();
}

static void test_ui_callbacks() {
    setup();
    GtkWidget *term1 = create_terminal_widget(NULL);
    gtk_box_pack_start(GTK_BOX(root_box), term1, TRUE, TRUE, 0);
    
    // Execute callbacks directly
    on_split_h(NULL, term1);
    on_split_v(NULL, term1);

    // Now safe because we mock the copy/paste
    on_copy(NULL, term1);
    on_paste(NULL, term1);
    
    // Close what we just opened
    GList *children = gtk_container_get_children(GTK_CONTAINER(root_box));
    if (children && children->data) {
        GtkWidget *paned = GTK_WIDGET(children->data);
        if (GTK_IS_PANED(paned)) {
            GtkWidget *nested_paned = gtk_paned_get_child1(GTK_PANED(paned));
            if (GTK_IS_PANED(nested_paned)) {
                GtkWidget *t1 = gtk_paned_get_child1(GTK_PANED(nested_paned));
                if (VTE_IS_TERMINAL(t1)) {
                    on_term_child_exited(VTE_TERMINAL(t1), 0, NULL);
                }
            }
        }
    }
    g_list_free(children);

    teardown();
}

static void test_button_press() {
    setup();
    GtkWidget *term1 = create_terminal_widget(NULL);
    gtk_box_pack_start(GTK_BOX(root_box), term1, TRUE, TRUE, 0);

    GdkEventButton event;
    event.type = GDK_BUTTON_PRESS;
    event.button = 3;
    // We pass NULL for GdkEvent* internally it just casts, so it may warn or we can just ignore
    on_button_press(term1, &event, NULL);
    
    // Test ignoring other buttons
    event.button = 1;
    on_button_press(term1, &event, NULL);
    event.type = GDK_BUTTON_RELEASE;
    on_button_press(term1, &event, NULL);

    teardown();
}

static void test_state_persistence() {
    setup();
    
    // Setup a specific layout
    GtkWidget *term1 = create_terminal_widget(NULL);
    gtk_box_pack_start(GTK_BOX(root_box), term1, TRUE, TRUE, 0);
    split_terminal(term1, 0, NULL); // horizontal
    
    // Theme
    current_theme_index = 1;
    
    save_state();
    
    // Verify file exists
    char path[512];
    snprintf(path, sizeof(path), "%s/.config/cli/state.txt", g_get_home_dir());
    FILE *f = fopen(path, "r");
    assert(f != NULL);
    
    char line[1024];
    fgets(line, sizeof(line), f);
    assert(strncmp(line, "THEME 1", 7) == 0);
    fclose(f);

    // Test load_state path
    teardown();
    setup();
    load_state();
    
    // Now corrupt the state to trigger the fallback mechanisms
    f = fopen(path, "w");
    fprintf(f, "THEME 999\n"); // invalid theme index
    fprintf(f, "INVALID\n");
    fclose(f);

    teardown();
    setup();
    load_state(); // Should fallback

    // Delete the file to cover the missing file fallback
    remove(path);
    teardown();
    setup();
    load_state();
    
    teardown();
}

static void test_initialization() {
    setup();
    apply_custom_css();
    load_custom_font();
    
    GtkWidget *term1 = create_terminal_widget(NULL);
    gtk_box_pack_start(GTK_BOX(root_box), term1, TRUE, TRUE, 0);
    
    gboolean found = FALSE;
    grab_first_terminal_focus(root_box, &found);
    assert(found == TRUE);

    // Call on_close_action to cover it
    on_close_action(NULL, term1);
    
    // Call on_app_quit
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
    
    printf("Running test_theme_application...\n");
    test_theme_application();

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

/* This file is part of PitchTempoPlayer (PTPlayer).

    PitchTempoPlayer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PitchTempoPlayer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PitchTempoPlayer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <gdk/gdkkeysyms.h>

#include "config.h"

#include "playlist.h"

/* 
The playlist is a store in which elements are added, removed or reorganized, by the selection file dialog or by drag-and-drop.
*/

gboolean firstPass = TRUE, selectionChangeCallback = TRUE;
int reorder = 0;
gint numberOfRows = 0;
char prefPath[100], last_selected_path[3];
GtkWidget *pListPlaylistView;
GList *removeRowList = NULL;

enum {
	IS_PLAYING_COLUMN,
	TITLE_COLUMN,
	NAME_COLUMN,
	N_COLUMN
	};

PreviousValue Previous = {0, 0};

/* Callback that allows double click on the playlist to switch the playing on the new selected selection; this is managed in the getFileName() function */
gint click_event(GtkWidget *widget, GdkEventButton *event, gpointer func_data)
{
	if (event->type==GDK_2BUTTON_PRESS) {
		DOUBLE_CLICK = TRUE;
		if (loop != NULL){
			g_main_loop_quit(loop);
			}
		//printf("I feel double clicked with button \n");		
		}
	return FALSE;
}

gboolean on_key_press (GtkWidget * window,
			  GdkEventKey*	pKey,
			  gpointer userdata){
			  
   if (pKey->type == GDK_KEY_PRESS){
         
        //g_print("%i\n", pKey->keyval);

          switch (pKey->keyval)
		{
			case GDK_KEY_Return :
				DOUBLE_CLICK = TRUE;
				if (loop != NULL){
					g_main_loop_quit(loop);
					}
				break;
			case GDK_KEY_Escape :
				gtk_main_quit ();
				break;
			case GDK_KEY_Delete :
				removeRows();
				get_playlist_store_size();
				update_array_playlist();
				reorder = 0;
				//gtk_tree_model_foreach(GTK_TREE_MODEL(pListPlaylistStore), (GtkTreeModelForeachFunc) updatePlaylistPosition, NULL);
				//updateGListFromTree();
				break;
		}
	}

	return FALSE;
}

/* foreach func to print each component of store */
gboolean
  foreachPrintStore (GtkTreeModel *model,
                GtkTreePath  *path,
                GtkTreeIter  *iter,
                gpointer      user_data)
  {
	gchar *data;
	gtk_tree_model_get (model, iter,
		        TITLE_COLUMN, &data,
		        -1);
	//printf("** list Store : %s\n", data); 
	return FALSE;
  }

/* Print each component of store */
void printfStore(GtkTreeModel *model){
	 gtk_tree_model_foreach (model, (GtkTreeModelForeachFunc) foreachPrintStore, NULL);
}

/* foreach func to get size of store */
gboolean
  foreachGetSizeOfStore (GtkTreeModel *model,
                GtkTreePath  *path,
                GtkTreeIter  *iter,
                gint* size)
  {
	*size += 1;
	return FALSE;
  }

/* Get playlist store size */
void get_playlist_store_size (){
	GtkTreeModel *model;
	gint size = 0;
	model = GTK_TREE_MODEL(pListPlaylistStore);
	gtk_tree_model_foreach (model, (GtkTreeModelForeachFunc) foreachGetSizeOfStore, &size);
	playlistStoreSize = size;
	//printf("playlistStoreSize is %d\n", playlistStoreSize);
}

gboolean
  getRemoveList (GtkTreeModel *model, GtkTreePath  *path, GtkTreeIter  *iter, GtkTreeSelection *selection){
	int i = 0;
	g_assert ( &removeRowList != NULL );
	GtkTreeRowReference  *rowref;
	rowref = gtk_tree_row_reference_new(model, path);
	removeRowList = g_list_append(removeRowList, rowref);
    return FALSE;
}

/* Get the name of the file to be played corresponding to the playlistPosition variable */
gchar* get_nth_filename_from_store(int playlistPosition){
	gchar *path_str, *pathName = NULL; 
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean valid;
	model = GTK_TREE_MODEL(pListPlaylistStore);
	valid = gtk_tree_model_get_iter_first (model, &iter);
	while (valid){
		/* Walk through the list, reading each row */
		gchar *title;
		gtk_tree_model_get (model, &iter,
				  TITLE_COLUMN, &title,
				  -1);
		GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
		path_str = gtk_tree_path_to_string(path);
		if (playlistPosition == atoi(path_str)){
			pathName = (gchar*)title;
			g_print("pathName = %s\n", pathName);
			}
		valid = gtk_tree_model_iter_next (model, &iter);
		}
	return pathName;
}

/* Get the name of the file to be played */
void getFileName()
{	
	/* If there was a double click on the playlist, DOUBLE_CLICK is set to TRUE, and the playlistPosition is the one of the selected row */
	if (DOUBLE_CLICK){
		playlistPosition = atoi(last_selected_path);
		DOUBLE_CLICK = FALSE;
		}
	/* update array position; if playlist order has been changed or reorganized, it will be updated now */
	update_array_playlist();
	/* Get the name of the file coresponding to the playlistPosition variable */
	sFile = (gchar*)get_nth_filename_from_store(playlistPosition);
	/* Stop if the end of the playlist is reached */
	/*if (sFile == NULL){
		onStop();
		}*/
}

/* Add file(s) to the playlist store */
void insertFilenamesInStore (GSList *gslist){
	int i = 0;
	GtkTreeModel *model;
	GtkTreeIter   iter;
	model = GTK_TREE_MODEL(pListPlaylistStore);
	gint listSize = g_slist_length(gslist);
	//printf("%d file(s) added to playlist\n", listSize);
	for (i = 0; i < listSize ; i++) {
		gpointer data = g_slist_nth_data(gslist, (guint)i);
		GFile *gfile = g_file_new_for_path ((gchar*)data);
		gchar *name = g_file_get_basename (gfile);
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			TITLE_COLUMN, data, 
			NAME_COLUMN, name, 
			-1);
	}
	printfStore(model);
}

 /* Select audio file */
void select_file(GtkWidget *pWidget, gpointer data) 
{
	GSList *filenames;
	GtkWidget *pFileSelection;
	GtkWidget *pParent;
	pParent = GTK_WIDGET(data);

	pFileSelection = gtk_file_chooser_dialog_new("Select File",
				GTK_WINDOW(pParent),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				"Cancel", GTK_RESPONSE_CANCEL,
				"Open", GTK_RESPONSE_OK,
				NULL);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (pFileSelection), g_get_home_dir());
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (pFileSelection), TRUE);
	gtk_window_set_modal(GTK_WINDOW(pFileSelection), TRUE);

	switch(gtk_dialog_run(GTK_DIALOG(pFileSelection))){
		case GTK_RESPONSE_OK:
		       	filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (pFileSelection));
			insertFilenamesInStore(filenames);
			get_playlist_store_size();
			getFileName();
			default:
		    break;
		}
	gtk_widget_destroy(pFileSelection);
}

gboolean
  updatePlaylistPosition (GtkTreeModel *model,
                GtkTreePath  *path,
                GtkTreeIter  *iter,
                gpointer      user_data)
  {
	gchar* playing;
	gtk_tree_model_get (model, iter,
		        0, &playing,
		        -1);
	gchar *str_path = gtk_tree_path_to_string(path);
	if (playing != NULL){
		if (strcmp(playing, "->") == 0){
			playlistPosition = (guint)(atoi(str_path));
			return TRUE;
			}
		else{
			return FALSE;
			}
		}
	else{
		return FALSE;
		}
}

/* foreach func called to update the array position */ 
gboolean
  foreach_set_array (GtkTreeModel *model,
                GtkTreePath  *path,
                GtkTreeIter  *iter,
                gint* count)
  {
	gchar array[5];
	if (*count == playlistPosition){
		g_snprintf(array, sizeof(array), "->");
		}
	else {
		g_snprintf(array, sizeof(array), "");
		}
	gtk_list_store_set(GTK_LIST_STORE(model), iter, 
			IS_PLAYING_COLUMN, &array, 
			-1);	
	*count += 1;
	return FALSE;
  }

/* Update the array position */
void update_array_playlist(){
	gint count = 0;
	GtkTreeModel *model;
	model = GTK_TREE_MODEL(pListPlaylistStore);
	gtk_tree_model_foreach (model, (GtkTreeModelForeachFunc) foreach_set_array, &count);
}

/* Add or reorder data in playlist store by drag-and-drop : either insert data at a specific location ("if path_str != NULL"), or append it at the end of the list; data can added to the list or their order can just be changed ("reorder = 1");
*/
void view_onDragDataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata)
{
	GtkTreeModel *model;
	GtkTreeIter   iter;
	GtkTreePath *path;
	GtkTreeViewDropPosition pos;
	gchar *path_str;
	
	selectionChangeCallback = FALSE;
		
	model = GTK_TREE_MODEL(userdata);
	path = gtk_tree_path_new ();
	gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (pListPlaylistView),
					   x, y,
					   &path,
					   &pos);
	path_str = gtk_tree_path_to_string(path);
	//printf("DRAG SELECTED ROW : %s\n", path_str);
	if (path_str != NULL) {
		if (reorder){
			gint i = 0;
			GList *node1, *node2, *node3; 
			for ( (node1 = Previous.isPlayingList) && (node2 = Previous.titleList) && (node3 = Previous.nameList) && (i = 0);  node1 != NULL;  (node1 = node1->next) && (node2 = node2->next) && (node3 = node3->next) && i++){
				gtk_list_store_insert(GTK_LIST_STORE(model), &iter, (gint) atoi(path_str) + i);
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
					IS_PLAYING_COLUMN, (gchar*)node1->data,
					TITLE_COLUMN, (gchar*)node2->data,
					NAME_COLUMN, (gchar*)node3->data, 
					-1);
				}
			reorder = 0;
			removeRows();
			}
		else {
			gchar** uriList = gtk_selection_data_get_uris(seldata);
			gint i = 0;
			for(i = 0; uriList[i] != NULL; i++){
				GFile *file, *file2;
				gchar *properPath, *fileName;
				file = g_file_new_for_uri (uriList[i]);
				properPath = g_file_get_path (file);
				fileName = g_file_get_basename (file);
				//printf("properPath is : %s\n", properPath);
				g_object_unref (file);
				g_object_unref (file2);
				gtk_list_store_insert(GTK_LIST_STORE(model), &iter, (gint) atoi(path_str) + i);
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
					TITLE_COLUMN, properPath,
					NAME_COLUMN, fileName,
					-1);
				}
			}
		}
	else {
		if (reorder){
			GList *node1, *node2, *node3; 
			for ( (node1 = Previous.isPlayingList) && (node2 = Previous.titleList) && (node3 = Previous.nameList);  node1 != NULL;  (node1 = node1->next) && (node2 = node2->next) && (node3 = node3->next)){
				gtk_list_store_append(GTK_LIST_STORE(model), &iter);
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
					IS_PLAYING_COLUMN, (gchar*)node1->data,
					TITLE_COLUMN, (gchar*)node2->data, 
					NAME_COLUMN, (gchar*)node3->data, 
					-1);
				}
			reorder = 0;
			removeRows();
			}
		else {
			gchar** uriList = gtk_selection_data_get_uris(seldata);
			int i = 0;
			for(i = 0; uriList[i] != NULL; i++){
				GFile *file, *file2;
				gchar *properPath, *fileName;
				file = g_file_new_for_uri (uriList[i]);
				properPath = g_file_get_path (file);
				fileName = g_file_get_basename (file);
				gtk_list_store_append(GTK_LIST_STORE(model), &iter);
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
					TITLE_COLUMN, properPath, 
					NAME_COLUMN, fileName, 
					-1);
				}
			}
		}
	if (playing == 1){
		gtk_tree_model_foreach(GTK_TREE_MODEL(pListPlaylistStore), (GtkTreeModelForeachFunc) updatePlaylistPosition, NULL);
		}
	//printf("playlistPosition = %d\n", playlistPosition);
	selectionChangeCallback = TRUE;
	get_playlist_store_size ();
	if (playing == 0){
		getFileName();
		}
}

void on_drag_data_get    (GtkWidget *widget, GdkDragContext *drag_context,
                        GtkSelectionData *sdata, guint info, guint time,
                        gpointer user_data){
	reorder = 1;
}

void deleteList(){
	int i = 0;
	if (Previous.titleList != NULL){
		int listSize = g_list_length(Previous.titleList);
		for (i = 0; i < listSize; i++){
			GList* data = g_list_nth(Previous.titleList, (guint)0);
			Previous.titleList = g_list_remove_link(Previous.titleList, data);
			g_list_free_1(data);
			GList* data2 = g_list_nth(Previous.isPlayingList, (guint)0);
			Previous.isPlayingList = g_list_remove_link(Previous.isPlayingList, data2);
			g_list_free_1(data2);
			GList* data3 = g_list_nth(Previous.nameList, (guint)0);
			Previous.nameList = g_list_remove_link(Previous.nameList, data3);
			g_list_free_1(data3);
			}
		}
}

void getSelectionData(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data){
	int i = 0;
	gchar *path_str, *isPlaying, *title, *name;
	gtk_tree_model_get (model, iter, 
			IS_PLAYING_COLUMN, &isPlaying,
			TITLE_COLUMN, &title,
			NAME_COLUMN, &name,
		        -1);
	Previous.isPlayingList = g_list_append(Previous.isPlayingList, isPlaying);
	Previous.titleList = g_list_append(Previous.titleList, title);
	Previous.nameList = g_list_append(Previous.nameList, name);
}

/* Get the number of rows in the selection */
void getNumberOfRows(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data){
	numberOfRows += 1;
}

/* Compare if the new selection is the same than the previous one; if the selection is the same, 'selectionMatch' is set to TRUE */
void comparePath(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gboolean* selectionMatch){
	if(numberOfRows == 1){
		/* Get the last selected Path (in case double click is made, this last selection will be played) */
		sprintf(last_selected_path, "%s", gtk_tree_path_to_string(path));
		/* Compare the new selection */
		GList *node;
		for ( node = removeRowList;  node != NULL;  node = node->next ){
			GtkTreePath *pathRef;
			pathRef = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
			if (pathRef){
		   		gint comp = gtk_tree_path_compare (path, pathRef);
				if (comp == 0 && firstPass == TRUE){
					*selectionMatch = TRUE;
					//printf("sameSelection!\n");
					firstPass = FALSE;
		   			}
				}
			}
		}
}

/*  */
void on_selection_changed (GtkTreeSelection *treeselection, gpointer user_data){
	if (selectionChangeCallback == TRUE){
		int i = 0;
		gboolean selectionMatch = FALSE;
		numberOfRows = 0;
			
		/* Get the number of rows in the selection */
		gtk_tree_selection_selected_foreach(treeselection, getNumberOfRows, NULL);
		/* Compare if the new selection is the same than the previous one; if the selection is the same, 'selectionMatch' is set to TRUE */
		gtk_tree_selection_selected_foreach (treeselection, (GtkTreeSelectionForeachFunc) comparePath, &selectionMatch);
		if (selectionMatch == TRUE){
			reorder = 1;
			firstPass = FALSE;
			/* Keep previous multiple selection highlighted */
			GList *node;
			selectionChangeCallback = FALSE;
			for ( node = removeRowList; node != NULL; node = node->next ){
				GtkTreePath *path;
				path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
				if (path){
					gchar *path_str = gtk_tree_path_to_string(path);
					gtk_tree_selection_select_path (treeselection, path);
					}
				}
				selectionChangeCallback = TRUE;
			}
		else if (selectionMatch == FALSE) {
			firstPass = TRUE;
			int i = 0;
			deleteList();
			gtk_tree_selection_selected_foreach (treeselection, (GtkTreeSelectionForeachFunc) getSelectionData, NULL);
			/* Clear removeRowList */
			gint removeRowListSize = g_list_length(removeRowList);
			if (numberOfRows == 1){
				for (i = 0; i < removeRowListSize; i++){
					GList* data = g_list_nth(removeRowList, (guint)0);
					removeRowList = g_list_remove_link(removeRowList, data);
					g_list_free_1(data);
					}
				}
			gtk_tree_selection_selected_foreach(treeselection, (GtkTreeSelectionForeachFunc) getRemoveList, &treeselection);
			}
		}
}

void removeRows (void)
  {
	gint i = 0;
	GList *node;
	for ( node = removeRowList;  node != NULL;  node = node->next ){
		GtkTreePath *path;
		path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)node->data);
		gchar *path_str = gtk_tree_path_to_string(path);
		if (path){
	   		GtkTreeIter  iter;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(pListPlaylistStore), &iter, path)){
				gtk_list_store_remove(pListPlaylistStore, &iter);
	   			}
			}
		}
	//g_list_foreach(removeRowList, (GFunc) gtk_tree_row_reference_free, NULL);
  }





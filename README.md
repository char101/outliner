Taskware
================

Currently a simple task outliner.

## Overview

* Data is stored in a sqlite database so you can easily access it with your own favorite language if needed.
* Portable
* Cross-platform as supported by Qt.
* Item text is rendered as markdown using [hoedown](https://github.com/hoedown/hoedown)

## Outliner

* Tree outliner
  * Add sibling after current item (`Insert` or `a`)
  * Add sibling before current item (`Alt + Insert` or `A`)
  * Add child (`Ctrl + Insert` or `b`)
  * Delete item (`Del` or `d`)
  * Edit item (`F2` or `e`)
  * Increase item indent (`Ctrl + Right`)
  * Decrease item indent (`Ctrl + Left`)
  * Move item up/down (`Ctrl + Up/Down`)
* Toggle item checkable (`c`)
* Toggle item completed status (`Space`) or just check/uncheck the checkbox
* Toggle item cancelled status (`c`)
* Toggle item project status (`p`)
* Toggle item milestone status (`m`)
* Highlight item (`h`)
* Set item priority (`1` or `2` or `3` or `0` - no priority) - the priority is shown as a color bar on the left of the task
* Sort children (`s`) - children items will be sorted by priority and completed status
* Zoom - make current item root of the tree (`z`)
* Unzoom (`Z` or `Backspace`) or click on the breadcrumb
* `Enter` or `Double click` - zoom if current item is a project/milestone, edit otherwise

### Structure Outliner

On the left of the outliner is the tree showing structure of the outline. Only items marked as project/milestone will appear on the tree.

## Screenshot

![Screnshot](http://char101.github.io/outliner/images/screenshot.png)

![Screnshot](http://char101.github.io/outliner/images/screenshot2.png)

## Downloads

[Static single file windows exe](https://github.com/char101/outliner/releases/)

## Standard QTreeView Key Bindings

* `Up`  
  Moves the cursor to the item in the same column on the previous row. If the parent of the current item has no more rows to navigate to, the cursor moves to the relevant item in the last row of the sibling that precedes the parent.
* `Down`  
  Moves the cursor to the item in the same column on the next row. If the parent of the current item has no more rows to navigate to, the cursor moves to the relevant item in the first row of the sibling that follows the parent.
* `Left`  
  Hides the children of the current item (if present) by collapsing a branch.
* `Minus`  
  Same as LeftArrow.
* `Right`  
  Reveals the children of the current item (if present) by expanding a branch.
* `Plus`  
  Same as RightArrow.
* `Asterisk`  
  Expands all children of the current item (if present).
* `PageUp`  
  Moves the cursor up one page.
* `PageDown`  
  Moves the cursor down one page.
* `Home`  
  Moves the cursor to an item in the same column of the first row of the first top-level item in the model.
* `End`  
  Moves the cursor to an item in the same column of the last row of the last top-level item in the model.
* `F2`  
  In editable models, this opens the current item for editing. The Escape key can be used to cancel the editing process and revert any changes to the data displayed.

## Other

* Compiling this app requires `Qt 5.4.0` and a C++ compiler that supports `C++11` standard.

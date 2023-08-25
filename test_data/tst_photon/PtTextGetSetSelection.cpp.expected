int foo() {
    int start, end, len;
    len = widget->setSelection(&start, &end);
    widget->setSelection(&start, &end);
    int widgetSelLen1 = widget->selectedText().length();
    if (widgetSelLen1 != 0) {
        start = widget->selectionStart();
        end = start + widgetSelLen1;
    } else {
        start = widget->cursorPosition();
        end = widget->cursorPosition();
    }    int *start2, *end2, len2;
    len2 = widget->setSelection(start2, end2);
    widget->setSelection(start2, end2);
    len2 =widget->selectedText().length();
    if (len2 != 0) {
        *start2 = widget->selectionStart();
        *end2 = *start2 + len2;
    } else {
        *start2 = widget->cursorPosition();
        *end2 = widget->cursorPosition();
    }
}

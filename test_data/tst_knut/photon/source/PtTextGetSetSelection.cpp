int foo() {
    int start, end, len;
    len = PtTextSetSelection(widget, &start, &end);
    PtTextSetSelection(widget, &start, &end);
    PtTextGetSelection(widget, &start, &end);
    int *start2, *end2, len2;
    len2 = PtTextSetSelection(widget, start2, end2);
    PtTextSetSelection(widget, start2, end2);
    len2 = PtTextGetSelection(widget, start2, end2);
}

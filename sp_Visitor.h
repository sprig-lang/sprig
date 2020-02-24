#ifndef sp_Visitor_h
#define sp_Visitor_h

typedef struct sp_Visitor sp_Visitor;

struct sp_Visitor {
    void (*visit)(sp_Visitor* vis, void* val);
};

#endif
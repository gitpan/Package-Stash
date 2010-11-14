/*
 * This file was generated automatically by ExtUtils::ParseXS version 2.2206 from the
 * contents of Stash.xs. Do not edit this file, edit Stash.xs instead.
 *
 *	ANY CHANGES MADE HERE WILL BE LOST! 
 *
 */

#line 1 "Stash.xs"
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define NEED_newRV_noinc
#define NEED_sv_2pv_flags
#include "ppport.h"

#ifndef gv_fetchsv
#define gv_fetchsv(n,f,t) gv_fetchpv(SvPV_nolen(n), f, t)
#endif

#ifndef mro_method_changed_in
#define mro_method_changed_in(x) PL_sub_generation++
#endif

#ifdef newSVhek
#define newSVhe(he) newSVhek(HeKEY_hek(he))
#else
#define newSVhe(he) newSVpv(HePV(he, PL_na), 0)
#endif

#ifndef savesvpv
#define savesvpv(s) savepv(SvPV_nolen(s))
#endif

/* HACK: scalar slots are always populated on perl < 5.10, so treat undef
 * as nonexistent. this is consistent with the previous behavior of the pure
 * perl version of this module (since this is the behavior that perl sees
 * in all versions */
#if PERL_VERSION < 10
#define GvSVOK(g) (GvSV(g) && SvTYPE(GvSV(g)) != SVt_NULL)
#else
#define GvSVOK(g) GvSV(g)
#endif

#define GvAVOK(g) GvAV(g)
#define GvHVOK(g) GvHV(g)
#define GvCVOK(g) GvCVu(g) /* XXX: should this really be GvCVu? or GvCV? */
#define GvIOOK(g) GvIO(g)

/* see above - don't let scalar slots become unpopulated, this breaks
 * assumptions in core */
#if PERL_VERSION < 10
#define GvSetSV(g,v) do {               \
    SV *_v = (SV*)(v);                  \
    SvREFCNT_dec(GvSV(g));              \
    if ((GvSV(g) = _v ? _v : newSV(0))) \
        GvIMPORTED_SV_on(g);            \
} while (0)
#else
#define GvSetSV(g,v) do {               \
    SvREFCNT_dec(GvSV(g));              \
    if ((GvSV(g) = (SV*)(v)))           \
        GvIMPORTED_SV_on(g);            \
} while (0)
#endif

#define GvSetAV(g,v) do {               \
    SvREFCNT_dec(GvAV(g));              \
    if ((GvAV(g) = (AV*)(v)))           \
        GvIMPORTED_AV_on(g);            \
} while (0)
#define GvSetHV(g,v) do {               \
    SvREFCNT_dec(GvHV(g));              \
    if ((GvHV(g) = (HV*)(v)))           \
        GvIMPORTED_HV_on(g);            \
} while (0)
#define GvSetCV(g,v) do {               \
    SvREFCNT_dec(GvCV(g));              \
    if ((GvCV(g) = (CV*)(v))) {         \
        GvIMPORTED_CV_on(g);            \
        GvASSUMECV_on(g);               \
    }                                   \
    GvCVGEN(g) = 0;                     \
    mro_method_changed_in(GvSTASH(g));  \
} while (0)
#define GvSetIO(g,v) do {               \
    SvREFCNT_dec(GvIO(g));              \
    GvIOp(g) = (IO*)(v);                \
} while (0)

typedef enum {
    VAR_NONE = 0,
    VAR_SCALAR,
    VAR_ARRAY,
    VAR_HASH,
    VAR_CODE,
    VAR_IO,
    VAR_GLOB,  /* TODO: unimplemented */
    VAR_FORMAT /* TODO: unimplemented */
} vartype_t;

typedef struct {
    vartype_t type;
    SV *name;
} varspec_t;

static U32 name_hash, namespace_hash, type_hash;
static SV *name_key, *namespace_key, *type_key;

const char *vartype_to_string(vartype_t type)
{
    switch (type) {
    case VAR_SCALAR:
        return "SCALAR";
    case VAR_ARRAY:
        return "ARRAY";
    case VAR_HASH:
        return "HASH";
    case VAR_CODE:
        return "CODE";
    case VAR_IO:
        return "IO";
    default:
        return "unknown";
    }
}

I32 vartype_to_svtype(vartype_t type)
{
    switch (type) {
    case VAR_SCALAR:
        return SVt_PV; /* or whatever */
    case VAR_ARRAY:
        return SVt_PVAV;
    case VAR_HASH:
        return SVt_PVHV;
    case VAR_CODE:
        return SVt_PVCV;
    case VAR_IO:
        return SVt_PVIO;
    default:
        return SVt_NULL;
    }
}

vartype_t string_to_vartype(char *vartype)
{
    if (strEQ(vartype, "SCALAR")) {
        return VAR_SCALAR;
    }
    else if (strEQ(vartype, "ARRAY")) {
        return VAR_ARRAY;
    }
    else if (strEQ(vartype, "HASH")) {
        return VAR_HASH;
    }
    else if (strEQ(vartype, "CODE")) {
        return VAR_CODE;
    }
    else if (strEQ(vartype, "IO")) {
        return VAR_IO;
    }
    else {
        croak("Type must be one of 'SCALAR', 'ARRAY', 'HASH', 'CODE', or 'IO'");
    }
}

void _deconstruct_variable_name(SV *variable, varspec_t *varspec)
{
    char *varpv;

    if (!SvCUR(variable))
        croak("You must pass a variable name");

    varspec->name = sv_2mortal(newSVsv(variable));

    varpv = SvPV_nolen(varspec->name);
    switch (varpv[0]) {
    case '$':
        varspec->type = VAR_SCALAR;
        sv_chop(varspec->name, &varpv[1]);
        break;
    case '@':
        varspec->type = VAR_ARRAY;
        sv_chop(varspec->name, &varpv[1]);
        break;
    case '%':
        varspec->type = VAR_HASH;
        sv_chop(varspec->name, &varpv[1]);
        break;
    case '&':
        varspec->type = VAR_CODE;
        sv_chop(varspec->name, &varpv[1]);
        break;
    default:
        varspec->type = VAR_IO;
        break;
    }
}

void _deconstruct_variable_hash(HV *variable, varspec_t *varspec)
{
    HE *val;
    STRLEN len;

    val = hv_fetch_ent(variable, name_key, 0, name_hash);
    if (!val)
        croak("The 'name' key is required in variable specs");

    varspec->name = sv_2mortal(newSVhe(val));

    val = hv_fetch_ent(variable, type_key, 0, type_hash);
    if (!val)
        croak("The 'type' key is required in variable specs");

    varspec->type = string_to_vartype(HePV(val, len));
}

int _valid_for_type(SV *value, vartype_t type)
{
    svtype sv_type = SvROK(value) ? SvTYPE(SvRV(value)) : SVt_NULL;

    switch (type) {
    case VAR_SCALAR:
        return sv_type == SVt_NULL ||
               sv_type == SVt_IV   ||
               sv_type == SVt_NV   ||
               sv_type == SVt_PV   ||
               sv_type == SVt_RV;
    case VAR_ARRAY:
        return sv_type == SVt_PVAV;
    case VAR_HASH:
        return sv_type == SVt_PVHV;
    case VAR_CODE:
        return sv_type == SVt_PVCV;
    case VAR_IO:
        return sv_type == SVt_PVIO;
    default:
        return 0;
    }
}

HV *_get_namespace(SV *self)
{
    dSP;
    SV *ret;

    PUSHMARK(SP);
    XPUSHs(self);
    PUTBACK;

    call_method("namespace", G_SCALAR);

    SPAGAIN;
    ret = POPs;
    PUTBACK;

    return (HV*)SvRV(ret);
}

SV *_get_name(SV *self)
{
    dSP;
    SV *ret;

    PUSHMARK(SP);
    XPUSHs(self);
    PUTBACK;

    call_method("name", G_SCALAR);

    SPAGAIN;
    ret = POPs;
    PUTBACK;

    return ret;
}

void _expand_glob(SV *self, SV *varname)
{
    SV *name;

    name = newSVsv(_get_name(self));
    sv_catpvs(name, "::");
    sv_catsv(name, varname);

    /* can't use gv_init here, because it screws up @ISA in a way that I
     * can't reproduce, but that CMOP triggers */
    gv_fetchsv(name, GV_ADD, SVt_NULL);
    SvREFCNT_dec(name);
}

SV *_get_symbol(SV *self, varspec_t *variable, int vivify)
{
    HV *namespace;
    HE *entry;
    GV *glob;

    namespace = _get_namespace(self);
    entry = hv_fetch_ent(namespace, variable->name, vivify, 0);
    if (!entry)
        return NULL;

    glob = (GV*)(HeVAL(entry));
    if (!isGV(glob))
        _expand_glob(self, variable->name);

    if (vivify) {
        switch (variable->type) {
        case VAR_SCALAR:
            if (!GvSVOK(glob))
                GvSetSV(glob, newSV(0));
            break;
        case VAR_ARRAY:
            if (!GvAVOK(glob))
                GvSetAV(glob, newAV());
            break;
        case VAR_HASH:
            if (!GvHVOK(glob))
                GvSetHV(glob, newHV());
            break;
        case VAR_CODE:
            croak("Don't know how to vivify CODE variables");
        case VAR_IO:
            if (!GvIOOK(glob))
                GvSetIO(glob, newIO());
            break;
        default:
            croak("Unknown type in vivication");
        }
    }

    switch (variable->type) {
    case VAR_SCALAR:
        return GvSV(glob);
    case VAR_ARRAY:
        return (SV*)GvAV(glob);
    case VAR_HASH:
        return (SV*)GvHV(glob);
    case VAR_CODE:
        return (SV*)GvCV(glob);
    case VAR_IO:
        return (SV*)GvIO(glob);
    default:
        return NULL;
    }
}

#line 351 "Stash.c"
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(var) if (0) var = var
#endif

#ifndef PERL_ARGS_ASSERT_CROAK_XS_USAGE
#define PERL_ARGS_ASSERT_CROAK_XS_USAGE assert(cv); assert(params)

/* prototype to pass -Wmissing-prototypes */
STATIC void
S_croak_xs_usage(pTHX_ const CV *const cv, const char *const params);

STATIC void
S_croak_xs_usage(pTHX_ const CV *const cv, const char *const params)
{
    const GV *const gv = CvGV(cv);

    PERL_ARGS_ASSERT_CROAK_XS_USAGE;

    if (gv) {
        const char *const gvname = GvNAME(gv);
        const HV *const stash = GvSTASH(gv);
        const char *const hvname = stash ? HvNAME(stash) : NULL;

        if (hvname)
            Perl_croak(aTHX_ "Usage: %s::%s(%s)", hvname, gvname, params);
        else
            Perl_croak(aTHX_ "Usage: %s(%s)", gvname, params);
    } else {
        /* Pants. I don't think that it should be possible to get here. */
        Perl_croak(aTHX_ "Usage: CODE(0x%"UVxf")(%s)", PTR2UV(cv), params);
    }
}
#undef  PERL_ARGS_ASSERT_CROAK_XS_USAGE

#ifdef PERL_IMPLICIT_CONTEXT
#define croak_xs_usage(a,b)	S_croak_xs_usage(aTHX_ a,b)
#else
#define croak_xs_usage		S_croak_xs_usage
#endif

#endif

/* NOTE: the prototype of newXSproto() is different in versions of perls,
 * so we define a portable version of newXSproto()
 */
#ifdef newXS_flags
#define newXSproto_portable(name, c_impl, file, proto) newXS_flags(name, c_impl, file, proto, 0)
#else
#define newXSproto_portable(name, c_impl, file, proto) (PL_Sv=(SV*)newXS(name, c_impl, file), sv_setpv(PL_Sv, proto), (CV*)PL_Sv)
#endif /* !defined(newXS_flags) */

#line 403 "Stash.c"

XS(XS_Package__Stash_new); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_new)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "class, package_name");
    {
	SV *	class = ST(0);
	SV *	package_name = ST(1);
#line 350 "Stash.xs"
    HV *instance;
    HV *namespace;
    SV *nsref;
#line 422 "Stash.c"
	SV *	RETVAL;
#line 354 "Stash.xs"
    if (!SvPOK(package_name))
        croak("The constructor argument must be the name of a package");

    instance = newHV();

    if (!hv_store(instance, "name", 4, SvREFCNT_inc_simple_NN(package_name), 0)) {
        SvREFCNT_dec(package_name);
        SvREFCNT_dec(instance);
        croak("Couldn't initialize the 'name' key, hv_store failed");
    }
    namespace = gv_stashpv(SvPV_nolen(package_name), GV_ADD);
    nsref = newRV_inc((SV*)namespace);
    if (!hv_store(instance, "namespace", 9, nsref, 0)) {
        SvREFCNT_dec(nsref);
        SvREFCNT_dec(instance);
        croak("Couldn't initialize the 'namespace' key, hv_store failed");
    }

    RETVAL = sv_bless(newRV_noinc((SV*)instance), gv_stashsv(class, 0));
#line 444 "Stash.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Package__Stash_name); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_name)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "self");
    {
	SV *	self = ST(0);
#line 380 "Stash.xs"
    HE *slot;
#line 466 "Stash.c"
	SV *	RETVAL;
#line 382 "Stash.xs"
    if (!sv_isobject(self))
        croak("Can't call name as a class method");
    slot = hv_fetch_ent((HV*)SvRV(self), name_key, 0, name_hash);
    RETVAL = slot ? SvREFCNT_inc_simple_NN(HeVAL(slot)) : &PL_sv_undef;
#line 473 "Stash.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Package__Stash_namespace); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_namespace)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "self");
    {
	SV *	self = ST(0);
#line 393 "Stash.xs"
    HE *slot;
#line 495 "Stash.c"
	SV *	RETVAL;
#line 395 "Stash.xs"
    if (!sv_isobject(self))
        croak("Can't call namespace as a class method");
    slot = hv_fetch_ent((HV*)SvRV(self), namespace_key, 0, namespace_hash);
    RETVAL = slot ? SvREFCNT_inc_simple_NN(HeVAL(slot)) : &PL_sv_undef;
#line 502 "Stash.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Package__Stash_add_symbol); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_add_symbol)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2)
       croak_xs_usage(cv,  "self, variable, initial=NULL, ...");
    {
	SV *	self = ST(0);
	varspec_t	variable;
	SV *	initial;
#line 408 "Stash.xs"
    SV *name;
    GV *glob;
#line 527 "Stash.c"

    if (SvPOK(ST(1)))
        _deconstruct_variable_name(ST(1), &variable);
    else if (SvROK(ST(1)) && SvTYPE(SvRV(ST(1))) == SVt_PVHV)
        _deconstruct_variable_hash((HV*)SvRV(ST(1)), &variable);
    else
        croak("varspec must be a string or a hashref");

	if (items < 3)
	    initial = NULL;
	else {
	    initial = ST(2);
	}
#line 411 "Stash.xs"
    if (initial && !_valid_for_type(initial, variable.type))
        croak("%s is not of type %s",
              SvPV_nolen(initial), vartype_to_string(variable.type));

    name = newSVsv(_get_name(self));
    sv_catpvs(name, "::");
    sv_catsv(name, variable.name);

    if (items > 2 && (PL_perldb & 0x10) && variable.type == VAR_CODE) {
        int i;
        char *filename = NULL;
        I32 first_line_num = -1, last_line_num = -1;
        STRLEN namelen;
        SV *dbval;
        HV *dbsub;

        if ((items - 3) % 2)
            croak("add_symbol: Odd number of elements in %%opts");

        for (i = 3; i < items; i += 2) {
            char *key;
            key = SvPV_nolen(ST(i));
            if (strEQ(key, "filename")) {
                if (!SvPOK(ST(i + 1)))
                    croak("add_symbol: filename must be a string");
                filename = SvPV_nolen(ST(i + 1));
            }
            else if (strEQ(key, "first_line_num")) {
                if (!SvIOK(ST(i + 1)))
                    croak("add_symbol: first_line_num must be an integer");
                first_line_num = SvIV(ST(i + 1));
            }
            else if (strEQ(key, "last_line_num")) {
                if (!SvIOK(ST(i + 1)))
                    croak("add_symbol: last_line_num must be an integer");
                last_line_num = SvIV(ST(i + 1));
            }
        }

        if (!filename || first_line_num == -1) {
            if (!filename)
                filename = CopFILE(PL_curcop);
            if (first_line_num == -1)
                first_line_num = PL_curcop->cop_line;
        }

        if (last_line_num == -1)
            last_line_num = first_line_num;

        /* http://perldoc.perl.org/perldebguts.html#Debugger-Internals */
        dbsub = get_hv("DB::sub", 1);
        dbval = newSVpvf("%s:%d-%d", filename, first_line_num, last_line_num);
        if (!hv_store_ent(dbsub, name, dbval, 0)) {
            warn("Failed to update $DB::sub for subroutine %s",
                 SvPV_nolen(name));
            SvREFCNT_dec(dbval);
        }
    }

    /* GV_ADDMULTI rather than GV_ADD because otherwise you get 'used only
     * once' warnings in some situations... i can't reproduce this, but CMOP
     * triggers it */
    glob = gv_fetchsv(name, GV_ADDMULTI, vartype_to_svtype(variable.type));

    if (initial) {
        SV *val;

        if (SvROK(initial)) {
            val = SvRV(initial);
            SvREFCNT_inc_simple_void_NN(val);
        }
        else {
            val = newSVsv(initial);
        }

        switch (variable.type) {
        case VAR_SCALAR:
            GvSetSV(glob, val);
            break;
        case VAR_ARRAY:
            GvSetAV(glob, val);
            break;
        case VAR_HASH:
            GvSetHV(glob, val);
            break;
        case VAR_CODE:
            GvSetCV(glob, val);
            break;
        case VAR_IO:
            GvSetIO(glob, val);
            break;
        }
    }

    SvREFCNT_dec(name);
#line 637 "Stash.c"
    }
    XSRETURN_EMPTY;
}


XS(XS_Package__Stash_remove_glob); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_remove_glob)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "self, name");
    {
	SV *	self = ST(0);
	SV *	name = ST(1);
#line 512 "Stash.xs"
    hv_delete_ent(_get_namespace(self), name, G_DISCARD, 0);
#line 658 "Stash.c"
    }
    XSRETURN_EMPTY;
}


XS(XS_Package__Stash_has_symbol); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_has_symbol)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "self, variable");
    {
	SV *	self = ST(0);
	varspec_t	variable;
#line 519 "Stash.xs"
    HV *namespace;
    HE *entry;
    SV *val;
#line 681 "Stash.c"
	int	RETVAL;
	dXSTARG;

    if (SvPOK(ST(1)))
        _deconstruct_variable_name(ST(1), &variable);
    else if (SvROK(ST(1)) && SvTYPE(SvRV(ST(1))) == SVt_PVHV)
        _deconstruct_variable_hash((HV*)SvRV(ST(1)), &variable);
    else
        croak("varspec must be a string or a hashref");
#line 523 "Stash.xs"
    namespace = _get_namespace(self);
    entry = hv_fetch_ent(namespace, variable.name, 0, 0);
    if (!entry)
        XSRETURN_UNDEF;

    val = HeVAL(entry);
    if (isGV(val)) {
        GV *glob = (GV*)val;
        switch (variable.type) {
        case VAR_SCALAR:
            RETVAL = GvSVOK(glob) ? 1 : 0;
            break;
        case VAR_ARRAY:
            RETVAL = GvAVOK(glob) ? 1 : 0;
            break;
        case VAR_HASH:
            RETVAL = GvHVOK(glob) ? 1 : 0;
            break;
        case VAR_CODE:
            RETVAL = GvCVOK(glob) ? 1 : 0;
            break;
        case VAR_IO:
            RETVAL = GvIOOK(glob) ? 1 : 0;
            break;
        }
    }
    else {
        RETVAL = (variable.type == VAR_CODE);
    }
#line 721 "Stash.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_Package__Stash_get_symbol); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_get_symbol)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "self, variable");
    {
	SV *	self = ST(0);
	varspec_t	variable;
#line 560 "Stash.xs"
    SV *val;
#line 743 "Stash.c"
	SV *	RETVAL;

    if (SvPOK(ST(1)))
        _deconstruct_variable_name(ST(1), &variable);
    else if (SvROK(ST(1)) && SvTYPE(SvRV(ST(1))) == SVt_PVHV)
        _deconstruct_variable_hash((HV*)SvRV(ST(1)), &variable);
    else
        croak("varspec must be a string or a hashref");
#line 562 "Stash.xs"
    val = _get_symbol(self, &variable, 0);
    if (!val)
        XSRETURN_UNDEF;
    RETVAL = newRV_inc(val);
#line 757 "Stash.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Package__Stash_get_or_add_symbol); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_get_or_add_symbol)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "self, variable");
    {
	SV *	self = ST(0);
	varspec_t	variable;
#line 574 "Stash.xs"
    SV *val;
#line 780 "Stash.c"
	SV *	RETVAL;

    if (SvPOK(ST(1)))
        _deconstruct_variable_name(ST(1), &variable);
    else if (SvROK(ST(1)) && SvTYPE(SvRV(ST(1))) == SVt_PVHV)
        _deconstruct_variable_hash((HV*)SvRV(ST(1)), &variable);
    else
        croak("varspec must be a string or a hashref");
#line 576 "Stash.xs"
    val = _get_symbol(self, &variable, 1);
    if (!val)
        XSRETURN_UNDEF;
    RETVAL = newRV_inc(val);
#line 794 "Stash.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Package__Stash_remove_symbol); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_remove_symbol)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 2)
       croak_xs_usage(cv,  "self, variable");
    {
	SV *	self = ST(0);
	varspec_t	variable;
#line 588 "Stash.xs"
    HV *namespace;
    HE *entry;
    SV *val;
#line 819 "Stash.c"

    if (SvPOK(ST(1)))
        _deconstruct_variable_name(ST(1), &variable);
    else if (SvROK(ST(1)) && SvTYPE(SvRV(ST(1))) == SVt_PVHV)
        _deconstruct_variable_hash((HV*)SvRV(ST(1)), &variable);
    else
        croak("varspec must be a string or a hashref");
#line 592 "Stash.xs"
    namespace = _get_namespace(self);
    entry = hv_fetch_ent(namespace, variable.name, 0, 0);
    if (!entry)
        XSRETURN_EMPTY;

    val = HeVAL(entry);
    if (isGV(val)) {
        GV *glob = (GV*)val;
        switch (variable.type) {
        case VAR_SCALAR:
            GvSetSV(glob, NULL);
            break;
        case VAR_ARRAY:
            GvSetAV(glob, NULL);
            break;
        case VAR_HASH:
            GvSetHV(glob, NULL);
            break;
        case VAR_CODE:
            GvSetCV(glob, NULL);
            break;
        case VAR_IO:
            GvSetIO(glob, NULL);
            break;
        }
    }
    else {
        if (variable.type == VAR_CODE) {
            hv_delete_ent(namespace, variable.name, G_DISCARD, 0);
        }
    }
#line 859 "Stash.c"
    }
    XSRETURN_EMPTY;
}


XS(XS_Package__Stash_list_all_symbols); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_list_all_symbols)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "self, vartype=VAR_NONE");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	SV *	self = ST(0);
	vartype_t	vartype;

	if (items < 2)
	    vartype = VAR_NONE;
	else {
    if (!SvPOK(ST(1)))
	croak("vartype must be a string");
    vartype = string_to_vartype(SvPV_nolen(ST(1)));
	}
#line 629 "Stash.xs"
    if (vartype == VAR_NONE) {
        HV *namespace;
        HE *entry;
        int keys;

        namespace = _get_namespace(self);
        keys = hv_iterinit(namespace);
        EXTEND(SP, keys);
        while ((entry = hv_iternext(namespace))) {
            mPUSHs(newSVhe(entry));
        }
    }
    else {
        HV *namespace;
        SV *val;
        char *key;
        I32 len;

        namespace = _get_namespace(self);
        hv_iterinit(namespace);
        while ((val = hv_iternextsv(namespace, &key, &len))) {
            GV *gv = (GV*)val;
            if (isGV(gv)) {
                switch (vartype) {
                case VAR_SCALAR:
                    if (GvSVOK(val))
                        mXPUSHp(key, len);
                    break;
                case VAR_ARRAY:
                    if (GvAVOK(val))
                        mXPUSHp(key, len);
                    break;
                case VAR_HASH:
                    if (GvHVOK(val))
                        mXPUSHp(key, len);
                    break;
                case VAR_CODE:
                    if (GvCVOK(val))
                        mXPUSHp(key, len);
                    break;
                case VAR_IO:
                    if (GvIOOK(val))
                        mXPUSHp(key, len);
                    break;
                }
            }
            else if (vartype == VAR_CODE) {
                mXPUSHp(key, len);
            }
        }
    }
#line 940 "Stash.c"
	PUTBACK;
	return;
    }
}


XS(XS_Package__Stash_get_all_symbols); /* prototype to pass -Wmissing-prototypes */
XS(XS_Package__Stash_get_all_symbols)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "self, vartype=VAR_NONE");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	SV *	self = ST(0);
	vartype_t	vartype;
#line 686 "Stash.xs"
    HV *namespace, *ret;
    SV *val;
    char *key;
    I32 len;
#line 967 "Stash.c"

	if (items < 2)
	    vartype = VAR_NONE;
	else {
    if (!SvPOK(ST(1)))
	croak("vartype must be a string");
    vartype = string_to_vartype(SvPV_nolen(ST(1)));
	}
#line 691 "Stash.xs"
    namespace = _get_namespace(self);
    ret = newHV();

    hv_iterinit(namespace);
    while ((val = hv_iternextsv(namespace, &key, &len))) {
        GV *gv = (GV*)val;

        if (!isGV(gv)) {
            SV *keysv = newSVpvn(key, len);
            _expand_glob(self, keysv);
            SvREFCNT_dec(keysv);
        }

        switch (vartype) {
        case VAR_SCALAR:
            if (GvSVOK(val))
                hv_store(ret, key, len, newRV_inc(GvSV(gv)), 0);
            break;
        case VAR_ARRAY:
            if (GvAVOK(val))
                hv_store(ret, key, len, newRV_inc((SV*)GvAV(gv)), 0);
            break;
        case VAR_HASH:
            if (GvHVOK(val))
                hv_store(ret, key, len, newRV_inc((SV*)GvHV(gv)), 0);
            break;
        case VAR_CODE:
            if (GvCVOK(val))
                hv_store(ret, key, len, newRV_inc((SV*)GvCV(gv)), 0);
            break;
        case VAR_IO:
            if (GvIOOK(val))
                hv_store(ret, key, len, newRV_inc((SV*)GvIO(gv)), 0);
            break;
        case VAR_NONE:
            hv_store(ret, key, len, SvREFCNT_inc_simple_NN(val), 0);
            break;
        }
    }

    mPUSHs(newRV_noinc((SV*)ret));
#line 1018 "Stash.c"
	PUTBACK;
	return;
    }
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_Package__Stash); /* prototype to pass -Wmissing-prototypes */
XS(boot_Package__Stash)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
#if (PERL_REVISION == 5 && PERL_VERSION < 9)
    char* file = __FILE__;
#else
    const char* file = __FILE__;
#endif

    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(items); /* -W */
    XS_VERSION_BOOTCHECK ;

        newXS("Package::Stash::new", XS_Package__Stash_new, file);
        newXS("Package::Stash::name", XS_Package__Stash_name, file);
        newXS("Package::Stash::namespace", XS_Package__Stash_namespace, file);
        newXS("Package::Stash::add_symbol", XS_Package__Stash_add_symbol, file);
        newXS("Package::Stash::remove_glob", XS_Package__Stash_remove_glob, file);
        newXS("Package::Stash::has_symbol", XS_Package__Stash_has_symbol, file);
        newXS("Package::Stash::get_symbol", XS_Package__Stash_get_symbol, file);
        newXS("Package::Stash::get_or_add_symbol", XS_Package__Stash_get_or_add_symbol, file);
        newXS("Package::Stash::remove_symbol", XS_Package__Stash_remove_symbol, file);
        newXS("Package::Stash::list_all_symbols", XS_Package__Stash_list_all_symbols, file);
        newXS("Package::Stash::get_all_symbols", XS_Package__Stash_get_all_symbols, file);

    /* Initialisation Section */

#line 734 "Stash.xs"
    {
        name_key = newSVpvs("name");
        PERL_HASH(name_hash, "name", 4);

        namespace_key = newSVpvs("namespace");
        PERL_HASH(namespace_hash, "namespace", 9);

        type_key = newSVpvs("type");
        PERL_HASH(type_hash, "type", 4);
    }

#line 1071 "Stash.c"

    /* End of Initialisation Section */

#if (PERL_REVISION == 5 && PERL_VERSION >= 9)
  if (PL_unitcheckav)
       call_list(PL_scopestack_ix, PL_unitcheckav);
#endif
    XSRETURN_YES;
}

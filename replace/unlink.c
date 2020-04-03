/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long (* old_sys_unlink)(const char __user *filename);
asmlinkage long (* old_sys_unlink_e32)(const char __user *filename);

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_unlink(const char __user *filename){
    mm_segment_t fs_save;
    long or;
    fs_save = get_fs();
    set_fs(get_ds());
    or = old_sys_unlink((const char __user *)filename);
    set_fs(fs_save);
    //pr_dev("Open DDDDD %ld\n",or);
    return or;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static inline long kernel_unlink_e32(const char __user *filename){
    mm_segment_t fs_save;
    long or;
    fs_save = get_fs();
    set_fs(get_ds());
    or = old_sys_unlink_e32((const char __user *)filename);
    set_fs(fs_save);
    //pr_dev("Open DDDDD %ld\n",or);
    return or;
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_unlink_e32(const char __user *filename) {
    //if(!efs_netctrl_open_check(filename,flags,mode))return -EPERM;
    const char *redirected;
    int r = file_redirect_check(filename,&redirected,1);
    if(r == OPEN_REDIRECTED_ALLOC){
    long r1 = kernel_unlink_e32(redirected);
    kfree(redirected);
    return r1;
    }
    else if(r == OPEN_REDIRECTED){
    return  kernel_unlink_e32(redirected);
    }
    return old_sys_unlink_e32(filename);
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
asmlinkage long efs_sys_unlink(const char __user *filename) 
{
    const char *redirected;
    int r = file_redirect_check(filename,&redirected,0);
    if(r == OPEN_REDIRECTED_ALLOC){
    long r1 = kernel_unlink(redirected);
    kfree(redirected);
    return r1;
    }
    else if(r == OPEN_REDIRECTED){
    return kernel_unlink(redirected);
    }
    return old_sys_unlink(filename);
}

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

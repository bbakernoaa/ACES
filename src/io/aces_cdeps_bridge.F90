module aces_cdeps_bridge_mod
  use iso_c_binding
  use ESMF
  use cdeps_inline_mod
  implicit none

  ! Persistent native Fortran objects to avoid leaks and maintain state
  type(ESMF_GridComp), save :: f_gcomp
  type(ESMF_Clock),    save :: f_clock_init
  logical,             save :: is_initialized = .false.

contains

  subroutine aces_cdeps_init(c_gcomp, c_mesh, &
                            yy, mm, dd, h, min, s, &
                            yy_s, mm_s, dd_s, h_s, min_s, s_s, &
                            dt_sec, stream_path_c, rc) bind(C, name="aces_cdeps_init")
    type(c_ptr), value                :: c_gcomp
    type(c_ptr), value                :: c_mesh
    integer(c_int), value             :: yy, mm, dd, h, min, s
    integer(c_int), value             :: yy_s, mm_s, dd_s, h_s, min_s, s_s
    integer(c_int), value             :: dt_sec
    type(c_ptr), value                :: stream_path_c
    integer(c_int), intent(out)       :: rc

    ! Local Fortran ESMF types
    type(ESMF_GridComp) :: f_gcomp
    type(ESMF_Clock)    :: f_clock
    type(ESMF_Mesh)     :: f_mesh
    character(kind=c_char), pointer :: stream_path_ptr(:)
    character(len=256)  :: f_stream_path
    integer             :: i, f_rc

    ! 1. Convert C strings to Fortran strings
    call c_f_pointer(stream_path_c, stream_path_ptr, [256])
    i = 1
    f_stream_path = ""
    do while (stream_path_ptr(i) /= c_null_char .and. i <= 256)
       f_stream_path(i:i) = stream_path_ptr(i)
       i = i + 1
    end do

    ! 2. Create native Fortran objects
    ! We create a NEW GridComp in Fortran to ensure it's fully valid and has a valid VM context.
    f_gcomp = ESMF_GridCompCreate(name='ACES_CDEPS', rc=f_rc)

    ! Create the Mesh handle from the C pointer.
    ! ESMF handles are structs; the second 8-byte word is a "validity flag" (82949521 for Mesh/Grid).
    f_mesh  = transfer([transfer(c_mesh,  0_8), 82949521_8], f_mesh)

    ! Create a native Fortran Clock from components to ensure it matches the model time.
    block
        type(ESMF_Time) :: start_time, stop_time
        type(ESMF_TimeInterval) :: time_step
        call ESMF_TimeSet(start_time, yy=int(yy), mm=int(mm), dd=int(dd), &
                          h=int(h), m=int(min), s=int(s), rc=f_rc)
        call ESMF_TimeSet(stop_time, yy=int(yy_s), mm=int(mm_s), dd=int(dd_s), &
                          h=int(h_s), m=int(min_s), s=int(s_s), rc=f_rc)
        call ESMF_TimeIntervalSet(time_step, s=int(dt_sec), rc=f_rc)
        f_clock_init = ESMF_ClockCreate(time_step, start_time, stopTime=stop_time, rc=f_rc)
    end block

    is_initialized = .true.

    ! 3. Call the high-level Fortran interface
    print *, "ACES Bridge: Initializing CDEPS with native handles."
    print *, "ACES Bridge: Stream file: ", trim(f_stream_path)
    print *, "ACES Bridge: GridComp valid=", ESMF_GridCompIsCreated(f_gcomp)
    print *, "ACES Bridge: Clock valid=", ESMF_ClockIsCreated(f_clock_init)

    call cdeps_inline_init(f_gcomp, f_clock_init, f_mesh, trim(f_stream_path), f_rc)
    print *, "ACES Bridge: CDEPS initialization returned RC: ", f_rc

    rc = int(f_rc, c_int)
  end subroutine

  subroutine aces_cdeps_advance(yy, mm, dd, ss, rc) bind(C, name="aces_cdeps_advance")
    integer(c_int), value       :: yy, mm, dd, ss
    integer(c_int), intent(out) :: rc

    type(ESMF_Clock) :: f_clock
    type(ESMF_Time)  :: curr_time
    type(ESMF_TimeInterval) :: dt
    integer          :: f_rc

    ! For advance, CDEPS only uses the clock to get the current time.
    call ESMF_TimeSet(curr_time, yy=int(yy), mm=int(mm), dd=int(dd), s=int(ss), rc=f_rc)
    call ESMF_TimeIntervalSet(dt, s=1, rc=f_rc)
    f_clock = ESMF_ClockCreate(dt, curr_time, rc=f_rc)

    call cdeps_inline_advance(f_clock, f_rc)

    call ESMF_ClockDestroy(f_clock, rc=f_rc)
    rc = int(f_rc, c_int)
  end subroutine

  subroutine aces_cdeps_get_ptr(stream_idx, fldname_c, data_ptr_c, rc) bind(C, name="aces_cdeps_get_ptr")
    integer(c_int), value       :: stream_idx
    type(c_ptr), value          :: fldname_c
    type(c_ptr), intent(out)    :: data_ptr_c
    integer(c_int), intent(out) :: rc

    character(kind=c_char), pointer :: fldname_ptr(:)
    character(len=256) :: fldname
    real(ESMF_KIND_R8), pointer :: data_ptr(:)
    integer :: i, f_rc

    ! Convert C string
    call c_f_pointer(fldname_c, fldname_ptr, [256])
    i = 1
    fldname = ""
    do while (fldname_ptr(i) /= c_null_char .and. i <= 256)
       fldname(i:i) = fldname_ptr(i)
       i = i + 1
    end do

    call cdeps_get_field_ptr(int(stream_idx), trim(fldname), data_ptr, f_rc)
    rc = int(f_rc, c_int)

    if (f_rc == ESMF_SUCCESS .and. associated(data_ptr)) then
        data_ptr_c = c_loc(data_ptr(1))
    else
        data_ptr_c = c_null_ptr
    end if
  end subroutine

  subroutine aces_cdeps_finalize() bind(C, name="aces_cdeps_finalize")
    integer :: f_rc
    if (is_initialized) then
        print *, "ACES Bridge: Finalizing CDEPS and destroying native handles."
        call cdeps_inline_finalize(f_rc)
        call ESMF_ClockDestroy(f_clock_init, rc=f_rc)
        call ESMF_GridCompDestroy(f_gcomp, rc=f_rc)
        is_initialized = .false.
    end if
  end subroutine

  subroutine aces_get_mesh_from_field(c_field, c_mesh, rc) bind(C, name="aces_get_mesh_from_field")
    type(c_ptr), value          :: c_field
    type(c_ptr), intent(out)    :: c_mesh
    integer(c_int), intent(out) :: rc

    type(ESMF_Field) :: f_field
    type(ESMF_Mesh)  :: f_mesh
    type(ESMF_Grid)  :: f_grid
    integer          :: f_rc

    ! Reconstitute Field handle. Validity flag for Field/Clock is 76838410.
    f_field = transfer([transfer(c_field, 0_8), 76838410_8], f_field)

    f_rc = ESMF_SUCCESS
    call ESMF_FieldGet(f_field, mesh=f_mesh, rc=f_rc)

    if (f_rc /= ESMF_SUCCESS .or. .not. ESMF_MeshIsCreated(f_mesh)) then
        call ESMF_FieldGet(f_field, grid=f_grid, rc=f_rc)
        if (f_rc == ESMF_SUCCESS .and. ESMF_GridIsCreated(f_grid)) then
            f_mesh = ESMF_MeshCreate(f_grid, rc=f_rc)
        end if
    end if

    rc = int(f_rc, c_int)
    if (f_rc == ESMF_SUCCESS .and. ESMF_MeshIsCreated(f_mesh)) then
        c_mesh = transfer(f_mesh, c_null_ptr)
    else
        c_mesh = c_null_ptr
    end if
  end subroutine

end module

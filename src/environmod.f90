! Copyright (c) 2007-2018 Alberto Otero de la Roza
! <aoterodelaroza@gmail.com>,
! Ángel Martín Pendás <angel@fluor.quimica.uniovi.es> and Víctor Luaña
! <victor@fluor.quimica.uniovi.es>.
!
! critic2 is free software: you can redistribute it and/or modify
! it under the terms of the GNU General Public License as published by
! the Free Software Foundation, either version 3 of the License, or
! (at
! your option) any later version.
!
! critic2 is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
! GNU General Public License for more details.
!
! You should have received a copy of the GNU General Public License
! along with this program.  If not, see
! <http://www.gnu.org/licenses/>.

! Atomic environment class
module environmod
  use types, only: celatom, species
  implicit none

  private

  !> Atomic environment type.
  !> - The first ncell atoms correspond to the atoms in the main cell (c%ncel). 
  !> - For atom i in the environment,
  !>   %x - coordinates in the reduced cell (-0.5 to 0.5 if i = 1..ncell in crystals, 0 to 1 in mols).
  !>   %r - Cartesian coordinates corresponding to %x
  !>   %idx - id from the non-equivalent list
  !>   %cidx - id from the complete list
  !>   %lenv - atcel(%cidx)%x + %lenv = xr2x(%x)
  !>   %ir, %ic, %lvec - rotm(%ir,1:3) * at(%idx)%x + rotm(%ir,4) + cenv(%ic) + %lvec = xr2x(%x)
  type environ
     logical :: ismolecule !< is this a molecule or a crystal?
     real*8 :: dmax0 !< Environment contains all atoms within dmax0 of every point in the cell
     real*8 :: sphmax !< The reduced cell is circumscribed by a sphere of radius sphmax
     real*8 :: boxsize !< length of the region side (bohr)
     real*8 :: xmin(3), xmax(3) !< encompassing box (environment)
     real*8 :: xminc(3), xmaxc(3) !< encompassing box (main cell)
     real*8 :: x0(3) !< origin of the to-region transformation
     integer :: n = 0 !< Number of atoms in the environment
     integer :: ncell = 0 !< Number of atoms in the unit cell
     integer :: nregc(3) !< Number of regions that cover the unit cell
     integer :: nreg(3) !< Number of regions that cover the environment
     integer :: nmin(3), nmax(3) !< Minimum and maximum region id
     integer :: nregion !< number of regions
     integer :: nregs !< number of regions in the search arrays (iaddregs, rcutregs)
     real*8 :: m_x2xr(3,3) !< cryst. -> reduced cryst.
     real*8 :: m_xr2x(3,3) !< reduced cryst. -> cryst.
     real*8 :: m_c2xr(3,3) !< cart. -> reduced cryst.
     real*8 :: m_xr2c(3,3) !< reduced cryst. -> cart.
     integer, allocatable :: imap(:) !< atoms ordered by region, c2i(at(imap(1->n))%r) is ordered
     integer, allocatable :: nrlo(:) !< nrlo(ireg) = i, at(imap(i)) is the first atom in region ireg
     integer, allocatable :: nrhi(:) !< nrlo(ireg) = i, at(imap(i)) is the last atom in region ireg
     integer, allocatable :: iaddregs(:) !< integer addition for each search region
     real*8, allocatable :: rcutregs(:) !< rcut for each search region (sorted array)
     type(celatom), allocatable :: at(:) !< Atoms (first ncell in the main cell)
   contains
     procedure :: end => environ_end !< Deallocate arrays and nullify variables
     ! initialization routines
     procedure :: build_mol => environ_build_from_molecule !< Build an environment from molecule input
     procedure :: build_crys => environ_build_from_crystal !< Build an environment from molecule input
     ! coordinate transformations
     procedure :: xr2c !< Reduced crystallographic to Cartesian
     procedure :: c2xr !< Cartesian to reduced crystallographic
     procedure :: xr2x !< Reduced crystallographic to crystallographic
     procedure :: x2xr !< Crystallographic to reduced crystallographic
     procedure :: y2z !< Any to any (c,x,xr)
     procedure :: c2p !< Cartesian to region
     procedure :: p2i !< Region to integer region
     procedure :: c2i !< Cartesian to integer region
     ! calculation routines
     procedure :: nearest_atom !< Returns the ID of the atom nearest to a given point
     ! utility routines
     procedure :: report => environ_report !< Write a report to stdout about the environment.
  end type environ
  public :: environ

  ! module procedure interfaces
  interface
     module subroutine environ_end(e)
       class(environ), intent(inout) :: e
     end subroutine environ_end
     module subroutine environ_build_from_molecule(e,n,at,m_xr2c,m_x2xr)
       class(environ), intent(inout) :: e
       integer, intent(in) :: n
       type(celatom), intent(in) :: at(n)
       real*8, intent(in) :: m_xr2c(3,3)
       real*8, intent(in) :: m_x2xr(3,3)
     end subroutine environ_build_from_molecule
     module subroutine environ_build_from_crystal(e,nspc,spc,n,at,m_xr2c,m_x2xr,dmax0)
       class(environ), intent(inout) :: e
       integer, intent(in) :: nspc
       type(species), intent(in) :: spc(nspc)
       integer, intent(in) :: n
       type(celatom), intent(in) :: at(n)
       real*8, intent(in) :: m_xr2c(3,3)
       real*8, intent(in) :: m_x2xr(3,3)
       real*8, intent(in), optional :: dmax0
     end subroutine environ_build_from_crystal
     pure module function xr2c(e,xx) result(res)
       class(environ), intent(in) :: e
       real*8, intent(in)  :: xx(3)
       real*8 :: res(3)
     end function xr2c
     pure module function c2xr(e,xx) result(res)
       class(environ), intent(in) :: e
       real*8, intent(in)  :: xx(3)
       real*8 :: res(3)
     end function c2xr
     pure module function xr2x(e,xx) result(res)
       class(environ), intent(in) :: e
       real*8, intent(in)  :: xx(3)
       real*8 :: res(3)
     end function xr2x
     pure module function x2xr(e,xx) result(res)
       class(environ), intent(in) :: e
       real*8, intent(in)  :: xx(3)
       real*8 :: res(3)
     end function x2xr
     pure module function y2z(e,xx,icrd,ocrd) result(res)
       class(environ), intent(in) :: e
       real*8, intent(in)  :: xx(3)
       integer, intent(in) :: icrd, ocrd
       real*8 :: res(3)
     end function y2z
     pure module function c2p(e,xx) result(res)
       class(environ), intent(in) :: e
       real*8, intent(in)  :: xx(3)
       integer :: res(3)
     end function c2p
     pure module function p2i(e,xx) result(res)
       class(environ), intent(in) :: e
       integer, intent(in)  :: xx(3)
       integer :: res
     end function p2i
     pure module function c2i(e,xx) result(res)
       class(environ), intent(in) :: e
       real*8, intent(in)  :: xx(3)
       integer :: res
     end function c2i
     module subroutine nearest_atom(e,xp,icrd,nid,dist,lvec,nid0,id0,nozero)
       class(environ), intent(in) :: e
       real*8, intent(in) :: xp(3)
       integer, intent(in) :: icrd
       integer, intent(out) :: nid
       real*8, intent(out) :: dist
       integer, intent(out), optional :: lvec(3)
       integer, intent(in), optional :: nid0
       integer, intent(in), optional :: id0
       logical, intent(in), optional :: nozero
     end subroutine nearest_atom
     module subroutine environ_report(e)
       class(environ), intent(in) :: e
     end subroutine environ_report
  end interface

end module environmod
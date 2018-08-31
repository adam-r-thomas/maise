 MAISE
Module for Ab Initio Structure Evolution (MAISE) features 
<br /> * neural network-based description of interatomic interactions
<br /> * evolutionary optimization
<br /> * structure analysis
<br />
<br /> 1. [General info](#general-info)
<br /> 2. [Installation](#installation)
<br /> 3. [Input](#input)
<br /> 4. [Examples](#examples)
<br /> 5. [Setup input tag description](#setup-input-tag-description)


---
## General info

Current beta version 2.0 works on Linux platforms and combines 3 modules for describing, optimizing, and analyzing atomic structures.

1 The neural network (NN) module builds, tests, and uses NN models to describe interatomic interactions with near-ab initio accuracy at a low computational cost compared to density functional theory calculations.

With the primary goal of uning NN models to accelerate structure search, the main function of the module is to relax given structures. To simplify the NN application, we cloasely matched the input and output file formats with those used in the VASP software. Previously parameterized NN models available in the 'MODELS' directory have been generated and extensively tested for crystalline and/or nanostructured materials.

Users can create their own NN models with MAISE which are typically trained on density functional theory (DFT) total energy and atomic force data for relatively small structures. The generation of relevant and diverse andconfigurations is done separately with an 'evolutionary sampling' protocol detailed in our published work [1]. The code introduces a unique feature, 'stratified training', of how to build robust NNs for chemical systems with several elements [1]. NN models are developed in a hierarchical fashion, first for elements, then for binaries, and so on, which enables generation of reusable libraries for extended blocks in the periodic table. 

2 The implemented evolutionary algorithm (EA) enables an efficient identification of ground state configurations at a given chemical composition. Our studies have shown that the EA is particularly advantageous in dealing with large structures when no experimental structural input is available [2,3]. 

The searches can be performed for 3D bulk crystals, 2D films, and 0D nanoparticles. Population of structures can be generated either randomly or predefined based on prior information. Essential operations are 'crossover', when a new configuration is created based on two parent structures in the previous generation, and 'mutation', when a parent structure is randomly distorted. For 0D nanoparticles we have introduced a range of alternative evolution operations which will be described in an upcoming paper. 

3 The analysis functions include the comparion of structures based on the radial distribution function (RDF), the determination of the space group and the Wyckoff positions with an external ISOTROPY package, etc. In particular, the RDF-based structure dot product is essential for eliminating duplicate structures in EA searches and selecting different configurations in the pool of low-energy structures. 
<br />
<br /> [1] https://journals.aps.org/prb/abstract/10.1103/PhysRevB.95.014114
<br /> [2] https://journals.aps.org/prl/abstract/10.1103/PhysRevLett.109.075501
<br /> [3] https://journals.aps.org/prb/abstract/10.1103/PhysRevB.98.085131

---
## Installation

The code has been extensively tested on Linux platforms. We will appreciate the feedback on the installation and performance of the package on different platforms.

1 For full functionality, MAISE requires the [GSL library](https://www.gnu.org/software/gsl/) to be compiled and the [ISOTROPY package](http://stokes.byu.edu/iso/isotropy.php) (version Version 6, January 2018) to be installed prior to MAISE compilation.

Please copy libgsl.a, libgslcblas.a, and all gsl/.h into maise/lib subdirectory and specify the path to ISOTROPY in the maise/makefile (e.g., IPATH := ~/bin/isotropy)

2 By default, the code will be compiled for parallel execution with OpenMP. If you do not wish to compile the parallel version set 'SERIAL    ?= 1' in maise/makefile

3 Use 'make --jobs' for full compilation, 'make clean' for cleaning most relevant  objects, and 'make clean-all' for cleaning all objects.

---
## Input

Main input files that define a simulation are 'setup' with job settings, 'model' with NN parameters, 'basis' with the symmetry functions converting a structure into the NN input, and 'table' with typical chemical element sizes. The atomic structure is read from the 'POSCAR' file that follows the VASP format. 

<table>
  <tr>
    <td></td>
    <td align="center" colspan="2">EVOS</td>
    <td align="center" colspan="4">NNET</td>
    <td align="center"> CELL</td>
  </tr>
  <tr>
    <td align="center"></td>      <td align="center">SEARCH</td> <td align="center">EXAM</td> <td align="center">PARSE</td> <td align="center">TRAIN</td> <td align="center">TEST</td> <td align="center">SIMUL</td> <td align="center">EXAM</td>
  </tr>
  <tr>
    <td
    align="center"><a href="https://github.com/ak-lab/maise/blob/master/bin/setup">setup</td> <td align="center">+</td> <td align="center">+</td> <td align="center">+</td> <td align="center">+</td> <td align="center">+</td> <td align="center">+</td> <td align="center"> </td> 
  </tr>
  <tr>
    <td align="center"><a href="https://github.com/ak-lab/maise/blob/master/bin/INI/table">table</td> <td align="center">+</td> <td align="center"> </td> <td align="center"> </td> <td align="center">+</td> <td align="center"> </td> <td align="center"> </td> <td align="center"> </td>
  </tr>
  <tr>
    <td align="center"><a href="https://github.com/ak-lab/maise/blob/master/bin/INI/model">model</td> <td align="center"> </td> <td align="center"> </td> <td align="center"> </td> <td align="center">+*</td> <td align="center">+#</td> <td align="center">+#</td> <td align="center"> </td>
  </tr>
  <tr>
    <td align="center"><a href="https://github.com/ak-lab/maise/blob/master/bin/INI/basis">basis</td> <td align="center"> </td> <td align="center"> </td> <td align="center">+</td> <td align="center">$</td> <td align="center"> </td> <td align="center"> </td> <td align="center"> </td>
  </tr>
  <tr>
    <td colspan="8" class="divider"><hr /></td>
  </tr>
  <tr>
    <td align="center"><a href="http://stokes.byu.edu/iso/isotropy.php">ISO</td>   <td align="center"> </td> <td align="center">+</td> <td align="center"> </td> <td align="center"> </td> <td align="center"> </td> <td align="center"> </td> <td align="center">+</td>
  </tr>
  <tr>
    <td align="center"><a href="https://www.gnu.org/software/gsl/">GSL</td>   <td align="center"> </td> <td align="center"> </td> <td align="center"> </td> <td align="center">+</td> <td align="center"> </td> <td align="center">+</td> <td align="center"> </td>
  </tr>
  <tr>
    <td align="left" colspan="8">      *   for stratified training one needs to provide individual models
                                 <br />$  'basis' stored in the parsed directory is appended to 'model' at the end of the training
                                 <br />#  'model' has 'basis' pasted at the end once training is finished</td>
  </tr>
</table>

--- 
The structure examination and manupulation functions are run by calling maise with a flag:

```
maise -flag
```

The structure examination and manupulation functions are defined as

|Flag| Flag Description|
|:---:|:-|
| cxc|    compute dot product for POSCAR0 and POSCAR1 using RDF           |
| cmp|    compare RDF, space group, and volume of POSCAR0 and POSCAR1     |
| wyc|    convert POSCAR into str.cif, CONV, PRIM                         |
| cif|    convert str.cif into CONV and PRIM                              |
| rot|    rotate  a nanoparticle along eigenvectors of moments of inertia |
| dim|    find    whether POSCAR is periodic (3) or non-periodic (0)      |
| box|    reset   the box size for nanoparticles                          |
| sup|    make    a supercell specified by na x nb x nc                   |
| vol|    compute volume per atom for crystal or nano structures    |

## Examples

Directory 'examples/' has samples of maise jobs for parsing data, training neural networks, and simulating structures with neural network models. Eash example has a README file, a setup file with only relevant tags for the particular job, and reference output files for comparison. 

## Input tags by type
---
[Main job type selector](#main-job-type-selector)\
[Structure-enviroment](#structure-enviroment)\
[Main EVOS](#main-evos)\
[EVOS operations](#evos-operations)\
[EVOS crossover/mutation](#evos-crossover/mutation)\
[Molecular dynamics](#molecular-dynamics)\
[Species related](#species-related)\
[I/O](#i/o)\
[General model](#general-model)\
[Neural Network model](#neural-network-model)\
[Neural Network training](#neural-network-training)\
[Parsing](#parsing)\
[Cell relaxation](#cell-relaxation)

### Main job type selector
[JOBT](#jobt)
### Structure enviroment
[NMAX](#nmax) [MMAX](#mmax)
### Main EVOS         
[CODE](#code) [DENE](#dene) [KMSH](#kmsh) [LBOX](#lbox) [NDIM](#ndim) [NITR](#nitr) [NNJB](#nnjb) [NPOP](#npop) [RAND](#rand) [RUNT](#runt) [SEED](#seed) [SITR](#sitr) [TINI](#tini)  
### EVOS operations      
[BLOB](#blob) [CHOP](#chop) [INVS](#invs) [MATE](#mate) [MUTE](#mute) [PACK](#pack) [PLNT](#plnt) [REFL](#refl) [RUBE](#rube) [SWAP](#swap) [TETR](#tetr)   
### EVOS crossover/mutation 
[ACRS](#acrs) [ADST](#adst) [ELPS](#elps) [LCRS](#lcrs) [LDST](#ldst) [MCRS](#mcrs) [SCRS](#scrs) [SDST](#sdst)   
### Molecular dynamics             
[COPL](#copl) [DELT](#delt) [MOVI](#movi) [NSTP](#nstp) [THRM](#thrm) [TMAX](#tmax) [TMIN](#tmin) [TSTP](#tstp)     
### Species related 
[ASPC](#aspc) [NSPC](#nspc) [TSPC](#tspc) 
### I/O 
[COUT](#cout) [DATA](#data) [DEPO](#depo) [EVAL](#eval) [OTPT](#otpt) [WDIR](#wdir)
## Neural Network model
[MODT](#modt) [NCMP](#ncmp) [NNGT](#nngt) [NNNN](#nnnn) [NNNU](#nnnu) [NSYM](#nsym) 
## Neural Network training    
[FMRK](#fmrk) [LREG](#lreg) [NTRN](#ntrn) [NTST](#ntst) [TEFS](#tefs) [NPAR](#npar)
## Parsing
[EMAX](#emax) [FMAX](#fmax) [FMIN](#fmin) [VMAX](#vmax) [VMIN](#vmin) [MMAX](#mmax) 
## Cell Relaxation
[ETOL](#etol) [MINT](#mint) [MITR](#mitr) [PGPA](#pgpa) [RLXT](#rlxt) [STOP](#stop) [TIME](#time)

---
## Setup input tag description
---
| TAG | DESCRIPTION |
|:--|:---------|
| <a name="jobt"></a>JOBT | structure analysis   (00) use analysis tools specified by flags, evolutionary search  (10) run  (11) soft exit  (12) hard exit (13) analysis, cell simulation      (20) relaxation (21) molecular dynamics, data parsing (30) prepare inputs for NN training , NN training          (40) full training (41) stratified training|
| <a name="code"></a>CODE | Type of the code in use. (0) MAISE-INT (1) VASP-EXT (2) MAISE-EXT|
| <a name="modt"></a>MODT | Type of the interatomic potential in use:  (1)  NN  (3)  Sutton-Chen  (4) Gupta1) MLP (3) SC (4) GP|
| <a name="npar"></a>NPAR | Nnumber of cores for parallel NN training or cell simulation|
| <a name="mint"></a>MINT | The optimizer algorithm for the neural network training and the cell optimization. (gsl minimier type (0) BFGS2 (1) CG-FR (2) CG-PR (3) steepest descent |
| <a name="mitr"></a>MITR | Maximum number of the optimization steps; if the desired accuracy is not reached for NN training or cell opimization steps|
| <a name="rlxt"></a>RLXT | Cell optimization type (2) force only (3) full cell (7) volume (ISIF in VASP)|
| <a name="stop"></a>STOP | Number of cell optimization steps with atomic distances shorter than those set in table file's atom part.|
| <a name="etol"></a>ETOL | Error tolerance for training or cell optimization convergence|
| <a name="tefs"></a>TEFS | Training target value. (0) E (1) EF (2) ES (3) EFS (4) TOY|
| <a name="fmrk"></a>FMRK | Fraction of atoms will be parsed to use for EF or EFS trainings. From 0 to 1|
| <a name="cout"></a>COUT | Output type in the OUTCAR file in cell evaluation and optimization.|
| <a name="nmax"></a>NMAX | Maximum number of atoms in the unit cell.|
| <a name="mmax"></a>MMAX | Maximum number of neighbrs within the cutoff radius.|
| <a name="nspc"></a>NSPC | Number of element types for evolutionary search, parsing the data and neural network training.|
| <a name="tspc"></a>TSPC | Atomic number of the elements specified with NSPC tag.|
| <a name="aspc"></a>ASPC | Number of atoms of each element for the evolutionary search.|
| <a name="dscr"></a>DSCR | Descriptor type for parsing the structure. (1) Power spectrum (2) Behler-Parrinello (3) Classical potential|
| <a name="nsym"></a>NSYM | Number of the Behler-Parrinello symmetry functions for parsing the data, as will be detailed in the "basis" file.|
| <a name="ncmp"></a>NCMP | The lenght of the input vector of the neural network.|
| <a name="ntrn"></a>NTRN | Number of structure used for the training of the neural network. (+) Number of structures (-) Percentage of the dataset form 0 to 100|
| <a name="ntst"></a>NTST | Number of structure used for the testing the neural network during the training process. (+) Number of structures (-) Percentage of the dataset form 0 to 100|
| <a name="nnnn"></a>NNNN | Number of hidden layers in MLP neural network (does not inlcude input vector and output neuron).|
| <a name="nnnu"></a>NNNU | Number of neurons in hidden layers in MLP neural network.|
| <a name="nngt"></a>NNGT | Activation function type for the hidden layers' neurons. (0) linear (1) tanh|
| <a name="emax"></a>EMAX | Parse only this fraction of lowest-energy structures. From 0 to 1|
| <a name="fmax"></a>FMAX | Will not parse data with forces larger than this value.|
| <a name="vmin"></a>VMIN | Will not parse data with volume/atom smaller than this value.|
| <a name="vmax"></a>VMAX | Will not parse data with volume/atom  larger than this value.|
| <a name="ndim"></a>NDIM | Dimension of the unit cell in evolutionary search and cell optimization. (3) Crystal (2) Film (0) Particle|
| <a name="lbox"></a>LBOX | Box dimension for generating particles in evolutionary search in Angs. (will be ignored for crystals).|
| <a name="npop"></a>NPOP | Population size in the evolutionary search.|
| <a name="sitr"></a>SITR | Starting iteration in the evolutionary search. (0) start from random or specified structures|
| <a name="nitr"></a>NITR | Final number of iterations in the evolutionary search (should be larger than SITR).|
| <a name="tini"></a>TINI | Type of starting the evolutionary search when SITR=0.|
| <a name="time"></a>TIME | Mximum time for cell relaxation in evolutionary search and cell optimization.|
| <a name="pgpa"></a>PGPA | Pressure in GPa units.|
| <a name="dene"></a>DENE | Store distinct structures generated in evolutionary search in POOL/ once they are within this energy/atom (eV/atom) window from the ground state.|
| <a name="kmsh"></a>KMSH | K-mesh density used for VASP-EVOS. Suggested values: 0.30 for s/c, 0.05 for metals|
| <a name="seed"></a>SEED | Starting seed for the random number generator in evolutionary search. (0) Uses time as seed (+) The seed value|
| <a name="rand"></a>RAND | Starting seed for the parsing of the dataset. (0) Uses time as seed (+) The seed value (-) No randomization: structures are parsed in listing order|
| <a name="tmin"></a>TMIN | Minimum temperature in MD runs (K).|
| <a name="tmax"></a>TMAX | Maximum temperature in MD runs (K).|
| <a name="tstp"></a>TSTP | Temperature step in MD runs (K) in running form TMIN to TMAX.|
| <a name="delt"></a>DELT | Time step in the MD runs.|
| <a name="nstp"></a>NSTP | Number of steps per temperature in MD runs.|
| <a name="copl"></a>COPL | Coupling constant in *** thermostat for MD runs. Suggested: 25.0|
| <a name="movi"></a>MOVI | Number of steps after which a snapshot of structure will be saved during the MD run.|
| <a name="thrm"></a>THRM | Thermostat type for the MD runs. (0) Dynamic (1) NH_aleksey (2) NH_leapfrog (3) NH_verlet|
| <a name="depo"></a>DEPO | Path to the DFT datasets to be parsed.|
| <a name="data"></a>DATA | Location of the parsed data to parse or read for training (will be overwritten during parsing).|
| <a name="otpt"></a>OTPT | Directory for storing model parameters in the training process.|
| <a name="eval"></a>EVAL | Directory for model testing data.|
| <a name="wdir"></a>WDIR | Work directory for evolutionary search, MD runs, etc.|
| <a name="tetr"></a>TETR | Fraction of the structures generated randomly using tetris operation. From 0 to 1|
| <a name="plnt"></a>PLNT | Fraction of the structures generated from seeds. From 0 to 1|
| <a name="pack"></a>PACK | Fraction of the structures generated from closed-pack structures. From 0 to 1|
| <a name="blob"></a>BLOB | Fraction of the structures generated randomly using blob shape. From 0 to 1|
| <a name="mate"></a>MATE | Fraction of the structures generated by crossover using two halves from each parent. From 0 to 1|
| <a name="swap"></a>SWAP | Fraction of the structures generated by crossover using core and shell of parents. From 0 to 1|
| <a name="rube"></a>RUBE | Fraction of the structures generated by Rubik's cube operation. From 0 to 1|
| <a name="refl"></a>REFL | Fraction of the structures generated by symmetrization via reflection. From 0 to 1|
| <a name="invs"></a>INVS | Fraction of the structures generated by symmetrization via inversion. From 0 to 1|
| <a name="chop"></a>CHOP | Fraction of the structures generated by chopping to make facets. From 0 to 1|
| <a name="mute"></a>MUTE | Fraction of the structures generated by random distortions to the structure. From 0 to 1|
| <a name="mcrs"></a>MCRS |  0.50              mutation rate in crossover|
| <a name="scrs"></a>SCRS |  0.00              crossover:  swapping rate|
| <a name="lcrs"></a>LCRS |  0.00              crossover:  mutation strength for lattice vectors|
| <a name="acrs"></a>ACRS |  0.10              crossover:  mutation strength for atomic positions|
| <a name="sdst"></a>SDST |  0.00              distortion: swapping rate|
| <a name="ldst"></a>LDST |  0.00              distortion: mutation strength for lattice vectors|
| <a name="adst"></a>ADST |  0.20              distortion: mutation strength for atomic positions|
| <a name="elps"></a>ELPS |  0.30              random:     nanoparticle ellipticity|

Simulating Schooling Behavior of Fish with Unreal Engine 5
==========================================================

Abstract
--------

In this project we use Unreal Engine 5 to simulate the schooling behavior of prey fishes as a flock. The prey flock performs different escaping or defensing maneuver in reaction of the predator’s movement and surrounding environments. The simulation is divided into multiple layers. The fundamental layer contains the basic rule of the prey flock that needs to be followed under all circumstances, which includes collision detection and avoidance, prey-to-prey perception and communi- cation system, and Boids algorithm for the prey fishes to maintain as a single flock. On top of that, we then implemented different maneuvers of the prey fish in reaction to the three different predator behaviors, which are predator-present, predator-chase, and predator-attack.

### Team Members (names in alphabetical order):

Bodong Wang, Penghai Wei, Xiaoxiang Zhao, Yuxin Wang  
005523428,     105726519,     205059635,     905129084  

Report
------

This is our project report in the local folder.  
[Project Report](./CS275_Project/CS_275_Report.pdf)

Source code
-----------

[https://github.com/charlixzhao/FishFlock](https://github.com/charlixzhao/FishFlock)

Note: The source codes and materials together are roughly 4Gbs, which is too big to upload to BruinLearn.  
Running the source code requires installing unreal engine 5.0. To run the source code, first clone the directory to your disk. Then right-click the FishFlock.uproject and click "Generate Visual Studio project files," which will generate a .sln project file in the current folder. Double click the generated .sln to open it with visual studio. Change the build target to FishFlock and then compile and run, which should open the unreal editor.

Executable Link
---------------

[https://drive.google.com/file/d/1jnnI7lPkXXNE5jlP09CD8YzGAhu9LOJc/view?usp=sharing](https://drive.google.com/file/d/1jnnI7lPkXXNE5jlP09CD8YzGAhu9LOJc/view?usp=sharing)

Note: the executable only supports Windows. Mac and Linux are not supported yet. Just download the .zip file and unzip it. Go into the Windows folder and double-click on FishFlock.exe, which should open the game.
To reset the game to its starting state, you must close the process and re-open the .exe file. Close the game by presing ALT+F4.

Representative Images
---------------------

The following images shows some key concepts or progress of our project:

### Environmental and Framework Images

This is the underwater environment we created for our fish eco-system.

![Underwater Environment](./images/HD_underwater_scene.png)

Underwater Environment

This image showcases our visual ray casting system.

![Visual Ray Casting](./images/vision.png)

Visual Ray Casting

### State Transition Logics

This image is a peak into our transition system between state and maneuvers.

![State Transition](./images/state_machine.jpg)

State Transition System

### Attack Avoidance Maneuver Concept Figure

The prey fishes (red) scatter toward all direction.

![Flash Maneuver](./images/flash.jpg)

Flash Maneuver Concept Figure

The prey fishes (red) divide into two groups, then escape and join behind the predator (black) in a circular trajectory.

![Fountain Maneuver](./images/fountain.jpg)

Fountain Maneuver Concept Figure

The prey fishes (red) are turning left, or have turned left and are running away from the predator (black).

![Hourglass Maneuver](./images/hourglass.jpg)

Hourglass Maneuver Concept Figure

The prey fishes (red) split into two group and follow the leader to escape toward opposite directions.

![Split Maneuver](./images/split.jpg)

Split Maneuver Concept Figure

Demo Video Collection
---------------------

Note: This is just a complete collection of demo videos. Please refer to the Report for the context of these demos and a detailed explanation of them. These links are also referenced in the Report.  
The source video files are too big to upload to BruinLearn so we upload them to YouTube. If there is issue opening the YouTube links please contact us and we will upload them to some file hosting sites.

**Fish Flock Behavior during Sequence of Attacks 1**  
  
  
**Fish Flock Behavior during Sequence of Attacks 2**  
  
  
**Predator Control**  
  
  
**Follow spline with collision avoidance**  
  
  
**Fast Avoid + Skitter Maneuver**  
  
  
**Ball Maneuver**  
  
  
**Hourglass Maneuver**  
  
  
**Herd Maneuver**  
  
  
**Fountain Maneuver**  
  
  
**Flash Maneuver**  
  
  
**Split Maneuver**
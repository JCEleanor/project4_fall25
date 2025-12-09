# Project4: Inter Process Communication Using FIFOs

In this project, you are going to extend the restricted shell, rsh, by adding a new built-in command to send messages to other users. The 13th command, **sendmsg**, can be uses as in the following example:

```
rsh>sendmsg user2 hello there!
```

The command above should send the message "hello there!" to user **user2**. Note that the message is not enclosed within quotes.

In this project, you will complete the missing pieces in the server and the client code. The places marked as "**TODO:**" described the expected behavious of the code snippet you will be adding at those locations.
Specifically, you will update **server.c** and **rsh.c** to obtain a working shell with messaging capabilities.

### Compiling, Running, and Testing Your Code

You can compile the server by typing **make server** and compile the client by typing **make rsh**. In order to test your code, you should first create 3 FIFOs like below:

```
mkfifo serverFIFO
mkfifo user1
mkfifo user2
```

These are the server and the client FIFOs. You can then run the server by typing "./server &" to run it in the background. Don't forget to kill the process after your finished with your testing. You can also run it in the foreground and run the client processes on different terminals.

In order to run a client, you execute rsh with the user name command line argument like below:

```
./rsh user1
```

You can then run a different client on a different terminal with username user2. Make sure your user names match the FIFOs you have created. If you have completed the implementation correctly, you should be able to send messages and see the messages arrive at the other user's rsh prompt.

This project will have only one test case, which is publicly available in the "test" directory. You can inspect the inputs and the expected outputs for each of the 3 processes in a session with a server and two clients.

### Github Repo Setup and Gradescope Submission Instructions

The setup of the git repo is similar to the one for Project 2 and Project 3. Refer to those instructions for your github repo setup. You can use the same ssh key you generated in Project 2 for all of your projects.

For Project 4, create a separate private repository specific for this project under your own user name. For example, I created a private repo named **csci210_project4** and my github username is tolgacan. A private repo can be created from the GitHub web site by going to your repositories after clicking on your profile picture (e.g., https://github.com/tolgacan?tab=repositories) and clicking on the "New" button in the top right corner. After that, you can enter a command sequence similar to the one below to clone the starter repo and copy it to your private repo to work with. The **project4_fall25** repo under the organization **CSCI210Mines** is the public repository that contains the code that you will be working on.

```
git clone git@github.com:CSCI210Mines/project4_fall25.git
cd project4_fall25
git remote remove origin
git remote add origin git@github.com:tolgacan/csci210_project4.git  # this is my private repo. replace it with your own private repo
git branch -M main
git push -u origin main
```

After these commands, you should have a copy of your starter code in your own repo and you can update **rsh.c** and **server.c** by completing the parts marked with "**TODO:**". You can compile the server by typing "make server" and compile the client by typing "make rsh". You can execute the following git commands to push your updates to your own repo:

```
git add rsh.c
git add server.c
git commit -m "your commit message here"
git push -u origin main
```

You can also clone your repo and work on it and push the updates from different machines if you add the ssh public keys for these machines to your github profile.

**Submission on Gradescope:**

After you are logged in to your github account in a browser, if you follow the Gradescope assignment page for this project from Canvas and try to upload a submission, you will be able to select the Github submission options and select your private repository for this project to submit your solution on Gradescope.

## Test

```
./rsh user1 < test/test_user1.in > user1_actual_output.txt &
./rsh user2 < test/test_user2.in > user2_actual_output.txt &


diff user1_actual_output.txt
```

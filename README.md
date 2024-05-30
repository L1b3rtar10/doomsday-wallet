# Doomsday-wallet
A safe, future-proof command-line wallet with minimal requirements: 
1. The availability of a minimal machine able to build and run C++ software.
2. User's knowledge.

Doomsday-wallet uses a secret fragmentation approach.
By applying Bitcoin's principle **trust no one** the user becomes the only responsible for their Bitcoin, avoiding the use of third-party hardware wallets or seed backup, that can be lost or used by an attacker to steal funds.

## Rationale
All hardware wallets comply with BIP39 in order to allow the user to recover their funds. This approach comes with some drawbacks:
1. Users end up with a word list that is not mnemonic, and must be backed up in order to recover funds.
2. Seed backup becomes a weakness, as anyone with access to the seed can generate the master key with access to all user's funds.
3. Access to funds depends on having a hardware wallet, that requires some type of configuration, relies on third-party software and compatibility between a host machine and the hardware itself.

## Software and hardware requirements
The source code is purposedly written in C++ to ensure it can be built on multiple platforms with the least amount of libraries (same libraries as Bitcoin core) and very little hardware requirements.

## Master key fragmentation
Doomsday wallet defines an algorithm to generate a 256 bit master key by recombining 3 different fragments (shards), that can only be recombined together by the user to restore their master key.
These fragments are:
1. Questions
2. OTP + Hashing algorithm
3. Answers

## Setup

### 1. Question list generation
The user creates a text file containing a random number of questions (questions.txt). These questions should be:

- Strictly personal: the answer can't be easily found online data, public accounts, etc... (i.e: where did you meet your wife?)
- Univocally answered: make sure you'll remember the answer, and the way it is spelt (letters casing, numbers). You might add a hint (preferably that only you can understand) as part of the question. (i.e: where did you live between 2000 and 2004? example answer: France)
- Time invariant: make sure the answer to each question does not change depending on the time, space or other variables.

The number of questions is up to the users. The more questions, the more steps and secrets are required to try guess the answers.

### 2. Hashing algorithm and OTP
The user generates and sets a random seed (OTP) before compiling the software.
**A true random seed should be preferred, guaranteeing that the final master key will be a true random number.**
The software implements a random bit generator that takes the user's answers to the questions' list as an input. As this could be easily cracked by a dictionary attack, the OTP guarantees that every iteration generates a true random number, that feeds the next iteration.

## Usage
When the software is run by passing the file name as an input parameter, it will parse the questions file and prompt the user with the questions they previously chose.
By answering each questions, the user will provide some additional randomicity to the hashing algorithm, that will **univocally** generate a master key, as long as the user answers in the exact same way.
At the end of the initialisation, the software provides some basic features to:
- Generate addresses
- Generate transactions
- Sign transactions

### Safety considerations
As a matter of fact, the user's master key does not exist in the real world and it is not available to anyone, so can't be used by an attacker.
Safety is achieved by splitting the key generation process into 3 different components, one of which is something only the user has access to (answers), so, again, not available to any external attacker.

A 256 bit **OTP** is embedded into the source code, but it doesn't need the extra level of safety that a BIP39 seed would.

Questions should be saved by the user and backed up, but they don't need the extra level of safety that a BIP39 seed would.

### Answers should not be saved anywhere.

The only precaution (although the possibility to exploit it is very narrow) is to not keep source code and questions on the same online cloud storage, as that will give out to a potential attacker most of the pieces of the puzzle, although answers **MUST** never be available online.
Running the software on an offline machine guarantees that the private key will not be accessible once generated.

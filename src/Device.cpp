//
// Created by Jan Heil on 13.08.2023.
//

#include <Arduino.h>
#include <vector>
#include "Device.h"
#include "commands/HelpCommand.h"
#include "commands/InfoCommand.h"
#include "commands/ByeCommand.h"
#include "commands/ResetCommand.h"
#include "commands/ClearCommand.h"

// Store Telnet Instance.
ESPTelnet telnet;
EscapeCodes ansi;

// Store Commands.
std::vector<Command *> commands;

/**
 * Begin Telnet Stream Server.
 */
void Device::beginTelnet() {
    // Add Telnet Listener.
    telnet.onConnect(Device::onConnect);
    telnet.onDisconnect(Device::onDisconnect);
    telnet.onInputReceived(Device::onInput);

    // Begin Telnet Server.
    telnet.begin();
}

/**
 * Println Message to Serial and Telnet.
 * @param dataIO
 */
void Device::println(String dataIO) {
    Serial.println(dataIO);
    telnet.println(dataIO);
}

/**
 * Print Message to Serial and Telnet.
 * @param dataIO
 */
void Device::print(String dataIO) {
    Serial.print(dataIO);
    telnet.print(dataIO);
}

/**
 * Write Message to Serial and Telnet.
 * @param dataIO
 */
void Device::write(String dataIO) {
    Serial.write(dataIO.toInt());
    telnet.print(dataIO.toInt());
}

/**
 * Handle Telnet Input Stream.
 */
void Device::handleTelnet() {
    // Handle Telnet Loop Function.
    telnet.loop();
}

/**
 * On Telnet Client Connect.
 * @param ipIO
 */
void Device::onConnect(String ipIO) {
    Serial.println("New Client Connected.");

    // Print Welcome Message to Client.
    telnet.print(ansi.cls());
    telnet.print(ansi.home());
    telnet.print(ansi.setFG(ANSI_BRIGHT_WHITE));
    telnet.print("\nWelcome " + telnet.getIP());
    telnet.print(" found ");
    telnet.print(commands.size());
    telnet.print((commands.size() == 1 ? " Command" : " Commands"));
    telnet.println(" :-)");
    telnet.println("(Use ^] + q  to disconnect.)");
    telnet.print(ansi.reset());
    telnet.print("\n\n> ");

    Command *command = new InfoCommand();
    Serial.println(command->invoke());
    delete command;
}

/**
 * On Telnet Data Input.
 * @param dataIO
 */
void Device::onInput(String dataIO) {
    Serial.println(dataIO);

    // Handle Command Input.
    Device::handleCommand(dataIO);

    // Print Shell Line.
    telnet.print("> ");
}

/**
 * On Telnet Client Disconnect.
 * @param addressIO
 */
void Device::onDisconnect(String addressIO) {
    Serial.println("Client has Disconnected.");
}

/**
 * Return Telnet Instance.
 * @return
 */
ESPTelnet Device::getTelnet() {
    return telnet;
}

/**
 * Return ANSI Codes Instance.
 * @return
 */
EscapeCodes Device::getANSI() {
    return ansi;
}

/**
 * Return registered Commands.
 * @return
 */
std::vector<Command *> Device::getCommands() {
    return commands;
}

/**
 * Add Command to Vector.
 * @param commandIO
 */
void Device::addCommand(Command *commandIO) {
    // Print Debug Message.
    Device::print("Added Command ");
    Serial.println(commandIO->invoke());
    Device::println(" to Device.");

    // Push Command into Vector.
    commands.push_back(commandIO);
}

/**
 * Register all Hard-Coded Commands.
 */
void Device::addCommands() {
    Device::addCommand(new HelpCommand());
    Device::addCommand(new InfoCommand());
    Device::addCommand(new ByeCommand());
    Device::addCommand(new ResetCommand());
    Device::addCommand(new ClearCommand());
}

/**
 * Handle Command Input.
 * @param dataIO
 */
void Device::handleCommand(String dataIO) {
    // Store Found State.
    bool foundIO = false;

    if (dataIO.startsWith("/")) {
        // Replace Slash Invoke.
        dataIO.replace("/", "");

        // Split via Spaces.
        std::vector <String> splitIO = Device::split(dataIO, " ");

        // Loop trough Commands and Check or Execute.
        for (Command *commandIO: Device::getCommands()) {
            // Check if Command exits/equals.
            if (String(splitIO[0]).equalsIgnoreCase(commandIO->invoke())) {

                // Delete first Vector Item.
                if (!splitIO.empty())
                    splitIO.erase(splitIO.begin());

                // Print Debug Message.
                Device::print("Args: ");
                Device::println(String(splitIO.size()));

                // Execute Commands.
                commandIO->execute(splitIO);

                // Override Found State.
                foundIO = true;
            }
        }

        // Print Error Message.
        if (!foundIO) {
            Device::print("Can't find Command use /help to Display all Commands.");
            Device::println(dataIO);
        }
    }
}
#pragma once
#include "ServerSystem.h"
#include "ByteStream.h"

namespace Funny
{
	/*
	* How do I unify this an a clean manner?
	* 
	* Creating and sending packets isn't as
	* simple as SendPacket and RecievePacket
	* as the sending a packet requires we know
	* what we want to send, hence having multiple
	* types of functions like SendEntity with the
	* entity of the given network ID being sent
	* using various serialization functions for
	* the different components. Recieving a packet
	* is different as we don't know what a packet
	* actually contains until we start to read it
	* for a prefixed type.
	* 
	* Is it really possible to unify packet read
	* and write to avoid the headache of having
	* to manage so many different cases of what
	* a packet could/should be interpreted as?
	* 
	* Actually, do we even need to have this class?
	* Wouldn't it make more sense to create a packet
	* struct that contains all the crap we should
	* be sending or recieving from the server?
	* 
	* Like, lets say we have a Transform packet with
	* an X, Y, and Z value. When we recieve a packet,
	* we pass all of it to a ByteSteam for processing.
	* We read through packet header crap to find the
	* ID of the thing we need to interpret this as,
	* being Transform. We then create a local Transform
	* packet struct to deserialize stuff in our stream
	* as Transform data through passing it in by ref.
	* We can then take the deserialized transform data
	* and apply it to a local component.
	* 
	* Same deal goes for sending a packet, except we
	* pass a ByteStream corresponding to the packet
	* we want to send into a Transform packet struct
	* to serialize the local component data we supply
	* to it prior into the packet's byte stream. Then
	* we toss the ByteStream into a Send call and the
	* reciever does the above.
	*/

	/*
	* Still unsure how I should handle making it clear
	* what a packet contains. Do I just serialize the
	* networkID so the server and client know what to
	* target, the update type so the clients know to
	* create, destroy, or update an entity, and the
	* signature of the entity so the clients know what
	* components they should be reading from the packet
	* to create and update an entity?
	* 
	* In that case, would I just check the entity's
	* signature one by one and add/update the component
	* for the given entity based on what bits are flipped?
	* And for a destroyed event we can just keep the
	* signature blank.
	*/

	/*
	* We might also need a server message packet to
	* tell the clients certain things like game state
	* updates (ie. a player won, which player won, who
	* has what score, etc.).
	*/
	enum PacketTypes
	{
		ENTITY_UPDATE,
		GAME_MESSAGE,
		CLIENT_INPUT
	};

	struct PacketHeader
	{
		PacketTypes packetType;

		void SerializePacket(ByteStream& outStream)
		{
			outStream.writeInt((int)packetType);
		}

		void DeserializePacket(ByteStream& inStream)
		{
			packetType = (PacketTypes)inStream.readInt();
		}
	};

	struct EntityPacketHeader
	{
		int networkID;
		NetUpdateType updateType;
		Signature signature;

		void SerializePacket(ByteStream& outStream)
		{
			outStream.writeInt(networkID);
			outStream.writeInt((int)updateType);
			outStream.writeBytes(&signature, sizeof(Signature));
		}

		void DeserializePacket(ByteStream& inStream)
		{
			networkID = inStream.readInt();
			updateType = (NetUpdateType)inStream.readInt();
			inStream.readBytes(&signature, sizeof(Signature));
		}
	};

	struct TransformPacket
	{
		Vector2 position
		{
			position.x = 0,
			position.y = 0
		};

		Vector2 scale
		{
			scale.x = 1,
			scale.y = 1
		};

		void SerializePacket(ByteStream& outStream)
		{
			outStream.writeFloat(position.x);
			outStream.writeFloat(position.y);
			outStream.writeFloat(scale.x);
			outStream.writeFloat(scale.y);
		}

		void DeserializePacket(ByteStream& inStream)
		{
			position.x = inStream.readFloat();
			position.y = inStream.readFloat();
			scale.x = inStream.readFloat();
			scale.y = inStream.readFloat();
		}
	};

	struct RenderablePacket
	{
		char name[20];
		int nameSize = 20;

		SDL_Rect sourceRect
		{
			sourceRect.x = 0,
			sourceRect.y = 0,
			sourceRect.w = 256,
			sourceRect.h = 256
		};

		ColorRGBA color
		{
			color.r = 255,
			color.g = 255,
			color.b = 255,
			color.a = 255
		};

		void SerializePacket(ByteStream& outStream)
		{
			outStream.writeString(name, nameSize);

			outStream.writeInt(sourceRect.x);
			outStream.writeInt(sourceRect.y);
			outStream.writeInt(sourceRect.w);
			outStream.writeInt(sourceRect.h);

			outStream.writeInt(color.r);
			outStream.writeInt(color.g);
			outStream.writeInt(color.b);
			outStream.writeInt(color.a);
		}

		void DeserializePacket(ByteStream& inStream)
		{
			nameSize = inStream.readString(name, 20);

			sourceRect.x = inStream.readInt();
			sourceRect.y = inStream.readInt();
			sourceRect.w = inStream.readInt();
			sourceRect.h = inStream.readInt();

			color.r = inStream.readInt();
			color.g = inStream.readInt();
			color.b = inStream.readInt();
			color.a = inStream.readInt();
		}
	};

	struct GameMessagePacket
	{
		char message[20];
		int messageLength;

		void SerializePacket(ByteStream& outStream)
		{
			outStream.writeString(message, messageLength);
		}

		void DeserializePacket(ByteStream& inStream)
		{
			messageLength = inStream.readString(message, 20);
		}
	};

	enum InputDirections
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		NO_INPUT
	};

	struct ClientInputPacket
	{
		InputDirections inputDir;

		void SerializePacket(ByteStream& outStream)
		{
			outStream.writeInt((int)inputDir);
		}

		void DeserializePacket(ByteStream& inStream)
		{
			inputDir = (InputDirections)inStream.readInt();
		}
	};
}
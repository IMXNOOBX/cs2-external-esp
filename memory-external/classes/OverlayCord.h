/*
 * https://github.com/SamuelTulach/OverlayCord
 */

#ifndef OVERLAYCORD_H
#define OVERLAYCORD_H

#include <Windows.h>
#include <iostream>
#include <string>

namespace OverlayCord
{
	namespace Communication
	{
		typedef struct _Header
		{
			UINT Magic;
			UINT FrameCount;
			UINT NoClue;
			UINT Width;
			UINT Height;
			BYTE Buffer[1];
		} Header;

		typedef struct _ConnectedProcessInfo
		{
			UINT ProcessId;
			HANDLE File;
			Header* MappedAddress;
		} ConnectedProcessInfo;

		inline bool ConnectToProcess(ConnectedProcessInfo& processInfo)
		{
			std::string mappedFilename = "DiscordOverlay_Framebuffer_Memory_" + std::to_string(processInfo.ProcessId);
			processInfo.File = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, mappedFilename.c_str());
			if (!processInfo.File || processInfo.File == INVALID_HANDLE_VALUE)
				return false;

			processInfo.MappedAddress = static_cast<Header*>(MapViewOfFile(processInfo.File, FILE_MAP_ALL_ACCESS, 0, 0, 0));
			return processInfo.MappedAddress;
		}

		inline void DisconnectFromProcess(ConnectedProcessInfo& processInfo)
		{
			UnmapViewOfFile(processInfo.MappedAddress);
			processInfo.MappedAddress = nullptr;

			CloseHandle(processInfo.File);
			processInfo.File = nullptr;
		}

		inline void SendFrame(ConnectedProcessInfo& processInfo, UINT width, UINT height, void* frame, UINT size)
		{
			// frame is in B8G8R8A8 format
			// size can be nearly anything since it will get resized
			// for the screen appropriately, although the maximum size is
			// game window width * height * 4 (BGRA)
			processInfo.MappedAddress->Width = width;
			processInfo.MappedAddress->Height = height;

			memcpy(processInfo.MappedAddress->Buffer, frame, size);

			processInfo.MappedAddress->FrameCount++; // this will cause the internal module to copy over the framebuffer
		}
	}

	namespace Drawing
	{
		typedef struct _Frame
		{
			UINT Width;
			UINT Height;
			UINT Size;
			void* Buffer;
		} Frame;

		typedef struct _Pixel
		{
			BYTE B, G, R, A;
		} Pixel;

		inline Frame CreateFrame(UINT width, UINT height)
		{
			Frame output;
			output.Width = width;
			output.Height = height;
			output.Size = width * height * 4;

			output.Buffer = malloc(output.Size);
			memset(output.Buffer, 0, output.Size);

			return output;
		}

		inline void CleanFrame(Frame& frame)
		{
			memset(frame.Buffer, 0, frame.Size);
		}

		inline void SetPixel(Frame& frame, UINT x, UINT y, Pixel color)
		{
			if (x < frame.Width && y < frame.Height)
			{
				Pixel* buf = static_cast<Pixel*>(frame.Buffer);
				buf[y * frame.Width + x] = color;
			}
		}

		inline void DrawLine(Frame& frame, UINT x1, UINT y1, UINT x2, UINT y2, Pixel color)
		{
			int dx = abs(static_cast<int>(x2 - x1)), sx = x1 < x2 ? 1 : -1;
			int dy = -abs(static_cast<int>(y2 - y1)), sy = y1 < y2 ? 1 : -1;
			int err = dx + dy, e2;

			while (true)
			{
				SetPixel(frame, x1, y1, color);
				if (x1 == x2 && y1 == y2)
					break;
				e2 = 2 * err;
				if (e2 >= dy) { err += dy; x1 += sx; }
				if (e2 <= dx) { err += dx; y1 += sy; }
			}
		}

		inline void DrawRectangle(Frame& frame, UINT x1, UINT y1, UINT width, UINT height, Pixel color)
		{
			for (UINT x = x1; x < x1 + width; x++)
			{
				SetPixel(frame, x, y1, color);
				SetPixel(frame, x, y1 + height - 1, color);
			}

			for (UINT y = y1; y < y1 + height; y++)
			{
				SetPixel(frame, x1, y, color);
				SetPixel(frame, x1 + width - 1, y, color);
			}
		}

		inline void DrawCircle(Frame& frame, UINT x0, UINT y0, UINT radius, Pixel color)
		{
			int x = radius;
			int y = 0;
			int radiusError = 1 - x;

			while (x >= y)
			{
				SetPixel(frame, x0 + x, y0 + y, color);
				SetPixel(frame, x0 - x, y0 + y, color);
				SetPixel(frame, x0 - x, y0 - y, color);
				SetPixel(frame, x0 + x, y0 - y, color);
				SetPixel(frame, x0 + y, y0 + x, color);
				SetPixel(frame, x0 - y, y0 + x, color);
				SetPixel(frame, x0 - y, y0 - x, color);
				SetPixel(frame, x0 + y, y0 - x, color);

				y++;
				if (radiusError < 0)
				{
					radiusError += 2 * y + 1;
				}
				else
				{
					x--;
					radiusError += 2 * (y - x + 1);
				}
			}
		}
	}
}

#endif
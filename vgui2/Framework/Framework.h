#pragma once
#include <wx/wx.h>
#include <wx/image.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>

const std::string FRAMEWORK_EMPTY_STRING = "";
const wxPoint FRAMEWORK_EMPTY_wxPOINT = wxPoint(0, 0);
//std::vector<std::string> FRAMEWORK_DEBUG;

namespace Framework
{
	class Debug
	{
	public:
		Debug() {};

		void Push(
			const std::string& What
		) {
			FRAMEWORK_DEBUG.push_back(What);
		}

		void LogTaggedInt(
			const std::string& _TAG, const int& _WHAT
		) {
			FRAMEWORK_DEBUG.push_back(
				"d_TaggedInt<" + _TAG + "> : " + std::to_string(_WHAT)
				);
		}

		void log_TaggedStdException(
			const std::string& _Tag, const std::exception& _Exception
		) {
			FRAMEWORK_DEBUG.push_back(
				"$exception<" + _Tag + ">: " + _Exception.what()
			);
		}

		const std::vector<std::string> Get()
		{
			return FRAMEWORK_DEBUG;
		}

		const void Clear()
		{
			this->FRAMEWORK_DEBUG = {};
		}

	private:
		std::vector<std::string> FRAMEWORK_DEBUG;
	};

	class System
	{ public: 
		class Clipboard
		{
		public:
			static bool Set(
				const std::string& _DATA
			) {
				if (!OpenClipboard(nullptr)) return false; /* Failed to open */

				/* Clear */
				EmptyClipboard();

				HGLOBAL _GLOBAL_HANDLER = GlobalAlloc(GMEM_MOVEABLE, _DATA.size() + 1);

				if (!_GLOBAL_HANDLER) { /* Failed to initialize */
					CloseClipboard(); return false;
				}

				/* Setup global memory lock */
				void* _GLOBAL_PTR = GlobalLock(_GLOBAL_HANDLER);

				if (_GLOBAL_PTR) { /* Copy memory */
					memcpy(_GLOBAL_PTR, _DATA.c_str(), _DATA.size() + 1);
					GlobalUnlock(_GLOBAL_HANDLER);
				}

				/* Finalize: set & close */
				SetClipboardData(CF_TEXT, _GLOBAL_HANDLER);
				CloseClipboard();
			};
		};
	};

	class Image
	{ public: 
		static wxImage cColor(
			wxImage _IMAGE, const wxColor& _COLOR, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
		) {
			/*
				Transformation 
			*/
			for ( // x axis 
				int width = 0; width < _IMAGE.GetWidth(); width++
				)
			{
				for ( // y axis 
					int height = 0; height < _IMAGE.GetHeight(); height++
					)
				{
					unsigned char* _PIXEL = (
						_IMAGE.GetData() + (height * _IMAGE.GetWidth() + width) * 3
						); // RGB Assumption 

					// Mod the color 
					_PIXEL[0] = _COLOR.Red(); 
					_PIXEL[1] = _COLOR.Green();
					_PIXEL[2] = _COLOR.Blue();
				}
			} // Transformation over, return given value 
			return _IMAGE;
		}

		static wxImage cSize( // High image quality is standard
			wxImage _IMAGE, const wxSize& _SIZE, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
		) {
			// Modify size 
			_IMAGE = _IMAGE.Scale(_SIZE.x, _SIZE.y, wxIMAGE_QUALITY_HIGH);

			return _IMAGE;
		}

	};


	class Transform
	{ public:
		struct ColoredLineHex
		{ public:
			ColoredLineHex(
				const std::string& _COLOR, const std::string& _TEXT
			) : _COLOR{ _COLOR }, _TEXT{ _TEXT }
			{

			}

			const std::string gColor() const { return _COLOR; }; 
			const std::string gText() const { return _TEXT; }
		public:
			const std::string _COLOR; 
			const std::string _TEXT; 
		};

		static const std::string RemoveAllColorEscapesFromAString(
			const std::string& _STRING
		) {
			std::string _STR_COPY = _STRING;
			while (true)
			{
				const size_t _FOUND_ESCAPE_AT = _STR_COPY.find("\\a");
				if (_FOUND_ESCAPE_AT
					== std::string::npos)
					break; // No escaped left | found at all 
				_STR_COPY.erase(_FOUND_ESCAPE_AT, 8);
			}
			return _STR_COPY; 
		}

		static const wxColor StringToHexColor(
			const std::string& _STRING
		) {
			/*
				Chat gpt spaggeti code but works fine and performance is quite goooooood :)
			*/
			unsigned long colorValue = std::stoul(_STRING, nullptr, 16);
			unsigned char red = (colorValue >> 16) & 0xFF;
			unsigned char green = (colorValue >> 8) & 0xFF;
			unsigned char blue = colorValue & 0xFF;

			return wxColour(red, green, blue);
		}

		static const std::vector<Framework::Transform::ColoredLineHex> StringToColoredLines(
			const std::string& _STRING
		) {
			std::vector<Framework::Transform::ColoredLineHex> _LINES;

			const std::string _ESCAPE = "\\a";
			const size_t _COLOR_CODE_LEN = 6; 

			std::string _STR_COPY = _STRING;

			// If anything is before first escape we need to push it to avoid data losssssssssss (im losing my life over this shit)
			const size_t _FOUND_FIRST_ESCAPE_AT = _STR_COPY.find(_ESCAPE);

			if (_FOUND_FIRST_ESCAPE_AT != 0)
			{
				_LINES.push_back(
					{ "DEFCLR", (_FOUND_FIRST_ESCAPE_AT == std::string::npos ? _STR_COPY : _STR_COPY.substr(0, _FOUND_FIRST_ESCAPE_AT)) }
				); } // We goooooood to gooooo 

			while (
				true
				)
			{
				const size_t _FOUND_ESCAPE_AT = _STR_COPY.find(_ESCAPE);

				if (_FOUND_ESCAPE_AT
					== std::string::npos)
					break; // No escape found (hahah no escape, no escape from coding this shit and making my life miserable hahaha(so funny)) 
				_STR_COPY = _STR_COPY.substr(_FOUND_ESCAPE_AT + _ESCAPE.length()); // Cut the str to resemble escape + escape '>' \a

				// Now whats left? colorcode + rest of the text 
				// what will we handle first? colorcode 
				const std::string _COLORCODE = _STR_COPY.substr(0, _COLOR_CODE_LEN);
				_STR_COPY = _STR_COPY.substr(_COLOR_CODE_LEN);
				// what we have left? rest of the string 
				// how do we locate end of current colored segment? by std::string::find 
				size_t _FOUND_NEXT_ESCAPE_AT = _STR_COPY.find(_ESCAPE);

				if (_FOUND_NEXT_ESCAPE_AT == std::string::npos)
				{ // Case: end of the line 
					_LINES.push_back({
						_COLORCODE, _STR_COPY
						}); break;
				}
				// Case: color codes are still present
				const std::string _TEXT = _STR_COPY.substr(0, _FOUND_NEXT_ESCAPE_AT);
				_STR_COPY = _STR_COPY.substr(_FOUND_NEXT_ESCAPE_AT);
				// What we have left? : nothing 
				// What we do? save and let the loop run again 

				_LINES.push_back({
					_COLORCODE, _TEXT
					});
			} // Whats up? We finalized main task, now return and enjoy pure performance issues caused by spaggeti code from above :)

			return _LINES;
		}

		static const std::vector<std::string> StringToVectorString(
			const std::string& Str)
		{
			if (Str.empty()) return {}; // Empty
			std::vector<std::string> OUTPUT;
			
			int Start = 0; int End; 

			while (
				(End = Str.find("\n", Start)) != std::string::npos
				) 
			{
				OUTPUT.emplace_back(Str.substr(
					Start, End - Start));
				Start = (End + 1);
			}

			if (Start < Str.size())
				OUTPUT.emplace_back(
					Str.substr(Start)
				); 
			return OUTPUT;
		};

		static const bool StringToBool(
			const std::string& Str, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
		) {
			if ( // True statement
				Str == "true"
				) return true; 
			if ( // False statement 
				Str == "false"
				) return false; 
			// Error occured (return false by default)
			_DEBUG_OBJ->Push("TRANSFORM_STRING_TO_BOOL_FAILED");
			return false; 
		}

		static const wxPoint WxSizeToWxPoint2D(
			const wxSize& _SIZE)
		{ return wxPoint(_SIZE.x, _SIZE.y); }

		static const wxSize WxPointToWxSize2D(
			const wxPoint& _POINT)
		{ return wxSize(_POINT.x, _POINT.y); }

		static const std::string StringVectorToString(
			const std::vector<std::string>& Vec
		)
		{
			if (Vec.empty()) return FRAMEWORK_EMPTY_STRING;

			size_t totalSize = 0;
			for (const auto& str : Vec) {
				totalSize += str.size() + 1;
			}

			std::string OUTPUT;
			OUTPUT.reserve(totalSize);

			// Concatenate all strings with a newline
			for (const std::string& VectorElement : Vec) {
				OUTPUT += VectorElement;
				OUTPUT += '\n';
			}

			// Remove the last newline
			if (!OUTPUT.empty()) {
				OUTPUT.resize(OUTPUT.size() - 1);  // Shrink by 1 to remove last \n
			}

			return OUTPUT;
		}
	};

	class Geometry
	{
	public:
		struct BoundingBox
		{
		public:
			BoundingBox(
				const wxPoint& _POSITION1, const wxPoint& _POSITION2
			) : _POSITION1{ _POSITION1 }, _POSITION2{ _POSITION2 } {

			};

			const wxPoint gStarting() { return _POSITION1; }
			const wxPoint gFinal() { return _POSITION2; }

			const wxSize gSize() { return Framework::Transform::WxPointToWxSize2D(_POSITION2 - _POSITION1); }

			const wxRect gRect() { return wxRect(_POSITION1, gSize()); }
		private:
			const wxPoint _POSITION1;
			const wxPoint _POSITION2;
		};

		static const bool cBound1D(
			const int& _POSITION,
			const int& _STARTING,
			const int& _SIZE
		) {
			if ((_POSITION >= _STARTING) && (_POSITION <= (_STARTING + _SIZE)))
				return true;
			return false; 
		}

		static inline const bool cBounds(
			const wxPoint& _POSITION,
			const wxPoint& _BOX_POSITION,
			const wxSize& _BOX_SIZE
		) {
			const int _MAX_X = _BOX_POSITION.x + _BOX_SIZE.x;
			const int _MAX_Y = _BOX_POSITION.y + _BOX_SIZE.y;

			return (_BOX_POSITION.x <= _POSITION.x && _POSITION.x <= _MAX_X) &&
				(_BOX_POSITION.y <= _POSITION.y && _POSITION.y <= _MAX_Y);
		}
	};

	class File
	{
	public:

		struct Buffer
		{ public:
			Buffer(
				const std::vector<std::string> Value = {}
			) : _VALUE{ Value } {};

			// [ GET ] 
			const std::string gString() const 
			{ return Framework::Transform::StringVectorToString(this->_VALUE); }

			const std::vector<std::string> gVector() const 
			{ return this->_VALUE; }

			// [ PROVIDE ] 
			const void pVector(
				const std::vector<std::string>& Value) 
			{ this->_VALUE = Value; };

			const void pString(
				const std::string& Value
			) { this->_VALUE = Framework::Transform::StringToVectorString(Value); }
		private:
			std::vector<std::string> _VALUE;
		};

		class Open
		{
		public:
			Open(const std::filesystem::path& _PATH) :
				_PATH{ _PATH }
			{

			}

			const void ProvidePath(
				const std::filesystem::path& _PATH
			) {
				this->_PATH = _PATH;
			};

			const bool Write(
				const Framework::File::Buffer& _DATA, const bool _VALIDATE = false, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
			) {
				if (this->_FILE
					.is_open() == true)
				{ // File opened, cannot proceed
					_DEBUG_OBJ->Push("FILE_IN_PROGRESS");
					return false;
				}

				try 
				{
					_FILE.open(_PATH,
						std::ios::out | std::ios::trunc);

					// Attempt To Write
					_FILE << _DATA.gString();

					if (_VALIDATE)
					{ // Check if written without corruption
						_FILE.close();

						if (this->Read().gString() == _DATA.gString())
						{ _DEBUG_OBJ->Push("FILE_VALIDATION_STATUS = TRUE"); return true; };

						_DEBUG_OBJ->Push("FILE_VALIDATION_STATUS = FALSE");
						return false; 
					}
					
				}
				catch (const std::fstream::failure& ex)
				{
					_DEBUG_OBJ->Push(ex.what());
					return false;
				}

				return true;
			}

			const Framework::File::Buffer Read(Framework::Debug* _DEBUG_OBJ = new Framework::Debug())
			{
				if (this->_FILE
					.is_open() == true)
				{ // File opened, cannot proceed
					_DEBUG_OBJ->Push("FILE_IN_PROGRESS");
					return {};
				}

				if (std::filesystem::is_regular_file(
					this->_PATH
				) != true)
				{ // File doesn't exist or is in invaild format
					_DEBUG_OBJ->Push("FILE_INVAILD");
					return {};
				}

				try
				{
					std::vector<std::string> BufferCollected; std::string BufferPacket; 

					_FILE.open(this->_PATH, std::ios::in);

					if (!_FILE) // Unknown file 
						throw std::exception(
							"FILE_UNKNOWN_EXIT"
						);

					// Iterate
					while (
						std::getline(_FILE, BufferPacket)
						) { BufferCollected.push_back(BufferPacket); }

					// Return 
					return Framework::File::Buffer(
						BufferCollected
					);
				}
				catch (
					const std::exception& ex
					) {
					_DEBUG_OBJ->Push(ex.what());
				}

				_DEBUG_OBJ->Push("FILE_SOMETHING_WENT_WRONG");
				return {};
			}

			const bool WriteAdditional(
				const Framework::File::Buffer& _DATA, const bool _VALIDATE = false, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
			) {
				if (this->_FILE
					.is_open() == true)
				{ // File opened, cannot proceed
					_DEBUG_OBJ->Push("FILE_IN_PROGRESS");
					return {};
				}

				// Collect buffers ;
				const std::string CurrentBuffer = this->Read().gString();

				const std::string BufferSum = ( // Add together buffers 
					CurrentBuffer + "\n" + _DATA.gString()
					);
				// Finalize 
				return this->Write(Framework::File::Buffer(
					Framework::Transform::StringToVectorString(BufferSum)), _VALIDATE
				);
			}
		
			const bool ModifyLine(
				const size_t& _AT, const std::string& _DATA, const bool _VALIDATE = false, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
			) {
				std::vector<std::string> Buffer = this->Read().gVector();

				if ( // Check if buffer has given placement 
					_AT <= Buffer.size() && 0 <= 0
					)
				{ // Error occured, push debug and abort 
					_DEBUG_OBJ->Push("INVAILD_MODIFIER_PLACEMENT"); return false; 
				}

				Buffer[ // Modify 
					_AT
				] = _DATA; 

				// Finalize 
				return this->Write(Framework::File::Buffer{Buffer}, _VALIDATE, _DEBUG_OBJ);
			}
		private:

			std::fstream _FILE;
			std::filesystem::path _PATH;
		};
	};

	class Config
	{ public:
		class File
		{ public:
			struct Element
			{ public:
				Element(const std::string& _NAME = "", const std::string& _VALUE = "") : _NAME{_NAME}, _VALUE{_VALUE}
				{}

				const std::string gName() const { return this->_NAME; };
				const std::string gValue() const { return this->_VALUE; };
			private:
				std::string _NAME; std::string _VALUE;
			};

			struct cValue
			{ public:
				cValue(const std::string& _VALUE, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()) : _VALUE{ _VALUE }, _DEBUG_OBJ{_DEBUG_OBJ}
				{
					if (_VALUE.empty()
						== true)
					{ // _VALUE is empty -> Error 
						_DEBUG_OBJ->Push("cVALUE_EMPTY"); Corrupted = true;
					} 
				};

				const int gInt(Framework::Debug* _DEBUG_OBJ = new Framework::Debug()) const
				{
					try {
						if (Corrupted == true)
							throw std::exception("TRIED_ACCESSING_CORRUPTED_cVALUE");
						return std::stoi(this->_VALUE);
					}
					catch (const std::exception& Ex) { _DEBUG_OBJ->Push(Ex.what()); }

					return -1;
				};

				const bool gBool(Framework::Debug* _DEBUG_OBJ = new Framework::Debug()) const
				{
					try {
						if (Corrupted == true)
							throw std::exception("TRIED_ACCESSING_CORRUPTED_cVALUE");
						return Framework::Transform::StringToBool(this->_VALUE);
					}
					catch (const std::exception& Ex) { _DEBUG_OBJ->Push(Ex.what()); }

					return false;
				};

				const std::string gString(Framework::Debug* _DEBUG_OBJ = new Framework::Debug()) const
				{
					try {
						if (Corrupted == true)
							throw std::exception("TRIED_ACCESSING_CORRUPTED_cVALUE");
						return this->_VALUE;
					}
					catch (const std::exception& Ex) { _DEBUG_OBJ->Push(Ex.what()); }

					return FRAMEWORK_EMPTY_STRING;
				};

			private:
				std::string _VALUE; bool Corrupted = false; 
				Framework::Debug* _DEBUG_OBJ;
			};

			File(
				const std::filesystem::path& _PATH
			) : _PATH{ _PATH } {

			};

			const void pPath(
				const std::filesystem::path& _PATH)
			{ this->_PATH = _PATH; }

			const bool cForCorruptedFileAndDirAndFix(Framework::Debug* _DEBUG_OBJ = new Framework::Debug())
			{
				const std::filesystem::path _DIR = _PATH.parent_path();

				if ( // Check for directories 
					std::filesystem::is_directory(_DIR) == false
					)
				{ // Create directories 
					if (std::filesystem::create_directories(
						_DIR
					) == false)
					{ // Failed -> return negative 
						_DEBUG_OBJ->Push(
							"FILE_CORRUPTION_FIX_FAILED -> CREATE_DIRECTORY_RETURNED_NEGATIVE"
						); return false; 
					} // Went well, proceed
				}

				if ( // File existance check 
					std::filesystem::is_regular_file(_PATH) == false 
					)
				{ // Write file -> empty (not expect validation) 
					return Framework::File::Open(_PATH).Write({}, false, _DEBUG_OBJ);
				} 
				// Not corrupted return positive 
				return true; 
			}

			const bool cForCorruptionAndFix(Framework::Debug* _DEBUG_OBJ = new Framework::Debug())
			{
				// GetFileBuffer 
				std::vector<std::string> FileBuffer = Framework::File::Open(_PATH).Read().gVector();

				if (
					FileBuffer.size() == 0
					) // Full corruption 
				{ // Override whole file 
					std::vector<std::string> BufferToWrite;
					for (
						const std::vector<std::string> ConfigElement : this->_SET
						)
					{ // Collect Buffer -> split actions for performance 
						BufferToWrite.push_back( gAssertedCollumStr(ConfigElement) );
					}

					if (BufferToWrite.empty() == true)
					{ // Check if empty 
						_DEBUG_OBJ->Push("_CORRUPTION_FIX_FAILED -> BUFFER_EMPTY"); return false; 
					}

					// Write (validation : true) 
					return Framework::File::Open(_PATH).Write(
						Framework::File::Buffer(BufferToWrite), true, _DEBUG_OBJ);
				}

				bool ReWriteFileBuffer = false; 
				for ( // Check for singular corruption 
					const std::vector<std::string> Element : this->_SET
					)
				{ // Perform file check
					bool FoundElementInFile = false; 

					size_t FileBufferAt = 0; 
					for (const std::string FileElement : FileBuffer)
					{ 
						const Framework::Config::File::Element ElementFormated = FormatRawElement(
							FileElement);

						if (ElementFormated.gName() == Element[0])
						{ // Name valid -> check if value is not empty 
							if (
								ElementFormated.gValue().empty() == true
								) // Value Empty
							{ // Fix corruption in FileBuffer -> Force rewrite after 
								FileBuffer
									[FileBufferAt] = gAssertedCollumStr(Element);
								ReWriteFileBuffer = true; 
							}
							FoundElementInFile = true; 
						}
						// Increment _AT;
						FileBufferAt++;
					}
					if (FoundElementInFile ==
						false)
					{ // Add element to FileBuffer in order to fix corruption -> Force rewrite after 
						FileBuffer.push_back(gAssertedCollumStr(Element)); ReWriteFileBuffer = true; 
					}
				}

				if ( // Check if ReWrite needed 
					ReWriteFileBuffer == true
					) 
				{ // Rewrite (validation: true)
					return Framework::File::Open(_PATH).Write(
						Framework::File::Buffer(FileBuffer), true
					);
				} return true; 
			}

			const bool qSetup( // Check for corruption and override _SET 
				const std::vector<std::vector<std::string>>& _SET, Framework::Debug* _DEBUG_OBJ = new Framework::Debug())
			{
				this->_SET = _SET;

				if ( // File and directory corruption fix 
					cForCorruptedFileAndDirAndFix() == false
					)
				{ // Fail 
					_DEBUG_OBJ->Push("FIX_DIR_AND_FILE_CORRUPTION -> FAIL"); return false;
				}

				if ( // File buffer corruption check
					cForCorruptionAndFix() == false
					)
				{ // Fail
					_DEBUG_OBJ->Push("FIX_BUFFER_CORRUPTION -> FAIL"); return false;
				}

				// Refresh buffer after all 
				return this->rBuffer();
			}

			const bool rBuffer(Framework::Debug* _DEBUG_OBJ = new Framework::Debug()) // Buffer refresh : Heavy operation (on larger data sets)
			{
				if ( // Fix corruption 
					cForCorruptionAndFix(_DEBUG_OBJ) == false
					)
				{ // Fail 
					_DEBUG_OBJ->Push("FIX_BUFFER_CORRUPTION -> FAIL"); return false;
				} 

				this->_BUFFER = Framework::File::Open(_PATH).Read(_DEBUG_OBJ).gVector();
				return true; 
			}

			const Framework::Config::File::cValue gValue(
				const std::string& _NAME, const bool _FIX_IF_ERROR = false, Framework::Debug* _DEBUG_OBJ = new Framework::Debug())
			{ // Iterate 
				for (const std::string& FileBufferSegment : _BUFFER)
				{ // Check for compatibility 
					Framework::Config::File::Element FileBufferSegmentElement = this->FormatRawElement(
						FileBufferSegment);

					if (FileBufferSegmentElement.gName()
						== _NAME)
					{ // Name is valid -> return cValue object 
						return Framework::Config::File::cValue(
							FileBufferSegmentElement.gValue()
						);
					}
				} // Failed to find -> push error
				_DEBUG_OBJ->Push("gVALUE_FAILED_TO_FIND");

				if ( // (check for corruption, and refresh buffer -> if wanted)
					_FIX_IF_ERROR == true
					) { cForCorruptionAndFix(_DEBUG_OBJ); rBuffer(_DEBUG_OBJ); };

				// Return empty string -> (cValue class will handle further erros)
				return { FRAMEWORK_EMPTY_STRING };
			}
		private:
			const std::string gAssertedCollumStr(
				const std::vector<std::string>& _PARAM, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
			) {
				/*
					"name":"value"
				*/

				if (_PARAM.size() != 2) 
					// Check if param has supported len
				{ _DEBUG_OBJ->Push("_PARAM_SIZE_NOT_SUPPORTED"); return FRAMEWORK_EMPTY_STRING; }

				if (_PARAM[0].empty() || _PARAM[1].empty()) 
					// Check if params aren't empty
				{ _DEBUG_OBJ->Push("EMPTY_PARAM_NOT_SUPPORTED"); return FRAMEWORK_EMPTY_STRING; }

				// Return str 
				return ("\"" + _PARAM[0] + "\":\"" + _PARAM[1] + "\"");
			}

			const std::string gCollumInside(
				const std::string& _STR, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
			) {
				if (_STR.empty()
					== true)
				{ // Empty String 
					_DEBUG_OBJ->Push("_STR_EMPTY"); return FRAMEWORK_EMPTY_STRING;
				}

				std::string c_STR = _STR;

				const size_t StrFirstCollum = c_STR.find(
					"\"");

				if (StrFirstCollum == std::string::npos)
				{ // Failed to find 1st collum
					_DEBUG_OBJ->Push("1_COLLUM_UNKNOWN"); return FRAMEWORK_EMPTY_STRING;
				}
				// Override with sub 
				c_STR = c_STR.substr(StrFirstCollum + 1);

				const size_t StrSecondCollum = c_STR.find(
					"\"");

				if (StrSecondCollum == std::string::npos)
				{ // Failed to find 2nd collum
					_DEBUG_OBJ->Push("2_COLLUM_UNKNOWN"); return FRAMEWORK_EMPTY_STRING;
				}

				// Final substr override
				c_STR = c_STR.substr(0, StrSecondCollum);

				return c_STR;
			}

			const Framework::Config::File::Element FormatRawElement(
				const std::string& _ELEMENT, Framework::Debug* _DEBUG_OBJ = new Framework::Debug()
			) {
				/*
					"name":"value"
				*/
				const size_t SeparatorPostion = _ELEMENT.find(":");

				if (SeparatorPostion == std::string::npos)
				{ // Failed to find separator
					_DEBUG_OBJ->Push("SEPARATOR_UNKNOWN"); return {};
				}

				// Get Params -> current format: "name" && "value"
				const std::string NameParam = _ELEMENT.substr(0, SeparatorPostion);
				const std::string ValueParam = _ELEMENT.substr(SeparatorPostion + 1);

				// Get Collums -> current format: name && value -> (final)
				const std::string Name = this->gCollumInside(NameParam);
				const std::string Value = this->gCollumInside(ValueParam);

				if ( // Empty check
					(Name.empty() == true) || (Value.empty() == true)
					) { // One of elements is empty 
					_DEBUG_OBJ->Push("2_COLLUM_UNKNOWN"); return {};
				}

				// Finalize with returning correct formats
				return Element{ Name, Value };
			}

			std::vector<std::string> _BUFFER; 
			std::filesystem::path _PATH; std::vector<std::vector<std::string>> _SET;
		};
	};

}
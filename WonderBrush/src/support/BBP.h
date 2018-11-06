// constants for the BBitmap Exchange Protocol
// You are allowed to use this protocol in your own apps,
// but please don't add to it yourself - contact bbp@sumware.demon.nl
// instead with your suggestions.

// Added for Becasso 1.5: optional entry_ref instead of BBitmap archive.
// If the BBP_OPEN_BBITMAP message has the "ref" entry, the BBP_SEND_BBITMAP
// reply message will contain the entry_ref to a saved graphic file.


const int32 BBP_OPEN_BBITMAP='BPob';	// External App -> BBitmap Editor App 
//		B_MESSENGER_TYPE "target" (the target to send the reply to)
//		B_MESSAGE_TYPE "BBitmap" (BBitmap archive)  *or*
//		B_REF_TYPE "ref" (entry_ref to the graphic file)
//		B_FLOAT_TYPE "zoom" (optional zooming factor e.g. 2.0f means 200%)
//		B_STRING_TYPE "name" (optional bitmap name, which can go in the window title )

const int32 BBP_REPLACE_BBITMAP='BPpb';	// External App -> BBitmap Editor App 
//		B_MESSAGE_TYPE "BBitmap" (BBitmap archive)
//		B_FLOAT_TYPE "zoom" (optional zooming factor e.g. 2.0f means 200%)
//		B_STRING_TYPE "name" (optional bitmap name, which can go in the window title )

const int32 BBP_BBITMAP_OPENED='BPbo';	// BBitmap Editor App -> External Messenger
//		B_MESSENGER_TYPE "target" (the editor window messenger)

const int32 BBP_SEND_BBITMAP='BPsb';	// BBitmap Editor Window -> External Messenger
//		B_MESSAGE_TYPE "BBitmap" (edited BBitmap archive) *or*
//		B_REF_TYPE "ref" (edited graphic file).

const int32 BBP_BBITMAP_CLOSED='BPbc';	// BBitmap Editor Window -> External Messenger

const int32 BBP_REQUEST_BBITMAP='BPrb';	// External Messenger -> BBitmap Editor Window

const int32 BBP_NO_WINDOWS='BPnw';	// Becasso -> External Messenger

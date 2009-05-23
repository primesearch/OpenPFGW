// simply here to make the "copy files" project work.

// make sure the "build" depends on all of the files needing copied (the headers at least)
// If any of these files change, the project gets rebuilt, and the post-build step (the actual
// copying of files) will take place.  This project does nothing more than that (but it does it
// automatically)

#pragma warning (disable:4100)

int main(int argc, char* argv[])
{
  return 0;
}


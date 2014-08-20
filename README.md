What is Inkscape?
=================

Inkscape is professional quality vector graphics software which runs on Windows, Mac OS X and Linux. It is used by design professionals and hobbyists worldwide, for creating a wide variety of graphics such as illustrations, icons, logos, diagrams, maps and web graphics. Inkscape uses the [W3C][1] open standard [SVG][2] (Scalable Vector Graphics) as its native format, and is free and open-source software.

[1]: <http://www.w3.org>

[2]: <http://www.w3.org/Graphics/SVG/>

Inkscape has sophisticated drawing tools with capabilities comparable to Adobe Illustrator, CorelDRAW and Xara Xtreme. It can import and export various file formats, including SVG, AI, EPS, PDF, PS and PNG. It has a [comprehensive feature set][3], a [simple interface][4], multi-lingual support and is designed to be extensible; users can customize Inkscape's functionality with [add-ons][5].

[3]: </about/features/>

[4]: </about/screenshots/>

[5]: </download/addons/>

The Inkscape project has a growing international [user community][6], and many [learning materials][7] exist to help get you started with your creations. [Help and support][8] is provided by the community, and there are lots of ways for you to [get involved][9] if you want to help improve the Inkscape project.

[6]: </community/>

[7]: </learn/>

[8]: </community/>

[9]: </community/>

Inkscape is a member of the [Software Freedom Conservancy][10], a US 501(c)(3) non-profit organization. Contributions to Inkscape are tax deductible in the United States.

[10]: <http://sfconservancy.org/>



This mirror
===========

This GitHub repository is an unofficial mirrory of Inkscape, which is hosted on Launchpad with Bazaar. To check out the code for yourself, do the following:

 - Set up a UbuntuOne account
 - Create a LaunchPad account linked to UbuntuOne
 - Install Bazaar (`bzr`)
 - Link Bazaar to LaunchPad (`bzr launchpad-login`)
 - Confirm your email address
 - Download with `bzr branch lp:inkscape`

This repository is for casual users that don't want to do all that. Here is how I created this mirror.

 - Download as above
 - `bzr update`
 - Hardlink into git working directory
 - `git push`
 - Set up .gitignore, update script

There is an issue tracker and pull requests are possible. You are welcome to use both of these, however they are not reviewed by anyone. Feel free to use them you want to document bugs but you are too lazy to set up a LaunchPad account to report bugs.

Please contact me at inkscape@phor.net if you need an update here, or if you want to help maintain this repo and bring in the Inkscape historical revisions.

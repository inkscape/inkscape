Customisable MessageBox Plug-In (v0.98 beta 3)

[  Archive Page: http://nsis.sourceforge.net/archive/???  ]

--------------

The Customisable MessageBox plug-in allows you to use a MessageBox in your installer which can be altered to how you want it to be.  You can control:
	*  the number of buttons shown
	*  the text each button shows
	*  a standard or customisable icon ( flexible so you can use the installer or other files )
	*  a fully working 'forth' button!

The plug-in uses a few tricks to allow for the messagebox functionality to be altered so that the button text can finally be altered making [ Yes | Yes to All | No | No to All ] ( and anything else you care for ) possible.

Also with the changes made it is now possible to have a functional 'forth' button - yes you read correctly!  When using the function just pass in four button texts and you will see the forth button - simple really :o)

There is only the one function to show the messagebox so enjoy.

[ As of v0.98 beta the parameters of calling the function have changed, check out usage for the revised options - thanks to n0On3 for the suggestions. ]


Usage
-----

messagebox::show  styles  caption  ( module_name, icon_id )  text   but1  [ but2  but3  but4 ]

styles - messagebox styles ( supports most of the windows messagebox styles )

caption - the text to be used for the dialog title ( or the installer title if not specified )

module_name - sets the name of the file ( usually a dll or exe file ) that contains the custom icon to be used - if 0 is passed then the installer will be used

icon_id - the id of the icon group to be used for the custom icon

text - the text to be shown by the messagebox

but1 - specified text or name id to use ie IDYES, IDNO, etc ( there must always be at least one button )

but2, but3, but4 - optional buttons which follow the same way as for but1


When passing in options, you should use a "" pair for options you pass in to ensure the strings are correctly read.  It is not necessary to do so for the predefined button texts
e.g. "IDCANCEL" is the same as IDCANCEL.

See Example.nsi for an example ;o)  ( bit hacked at the moment due to testing but shows most things )


Return Values
-----

To get the return value use Pop $0 immediately after the function has been used incase other functions alter the value.

If there were no problems then the function will return the number of the button pressed working from the left being 'button 1'.
i.e.
	if you have [ yes | no | abort ] and 'no' is selected then the return value will be '2'

If an error happens then the messagebox will return 0.

If there are no buttons passed then the function will return '-1'.


Some More Info
----------------

If you pass an empty string for the caption ( "" ) then the installer title text will be used instead (stripping back the current section name as NSIS messageboxes do).
If the installer is run silently then the title will be blank if setting the caption option to "" ( as NSIS messageboxes do ).  This is because when the installer is silent it does not have a visible window and so will not have a window title ( had not taken this into account initially, oops - fixed from v0.98 beta 3 ).

When you want to use a custom icon, setting module to '0' will look for the icon in the installer otherwise it will search the file specified.
e.g.
	"0,103" will load the installer icon ( well in testing it does :o)  )
	"shell32.dll,24" will load the help file icon from the shell32.dll ( as long as the file exists! )

If the module passed is not valid then the usericon style will not be used ( you may receive a windows message informing of this ).

If the icon is not valid then the messagebox will just show a blank area where the icon would be displayed assuming that the value of module is valid.

If a usericon is specified then if there are any of the messagebox icon styles passed in, these will be ignored.  This allows an icon to appear otherwise internal style conflicts will prevent any icon being shown.

If you want to display the standard button texts then pass the following strings in for the necessary button:

	IDOK		-	Ok
	IDCANCEL	-	Cancel
	IDABORT	-	Abort
	IDRETRY	-	Retry
	IDIGNORE	-	Ignore
	IDYES		-	Yes
	IDNO		-	No
	IDHELP		-	Help / 4th button
	IDTRYAGAIN	-	Try Again
	IDCONTINUE	-	Continue

When the function is processing the passed button texts to use, if a duplicate of the predefined texts (above) happen then the only the first instance will be allowed
e.g.
	IDYES "ah go on if you dare" IDYES	->	[  YES  |  ah go on if you dare  ]

Each button will be resized if needed to allow the text to fit correctly in the button without being clipped.  If the text is still too long for the messagebox width ( limited to 80% of the screen width ) then buttons will be resized and clipping of the button text may happen again ( this will be fixed in v1.0 ).


Final Notes
-------

I have no idea how the code will work with international text (since i just use plain text in the code) so if you try it in a multilanguage setup then let me know how it reacts or if there are any issues with the function's display.

The predefined button texts are hard coded and cannot be altered.  To alter the default texts involves passing in the language button text as a custom button text.

The button resizing code is not complete at the moment ( as can be clearly seen from the first messagebox ) when it detects that the width of the messagebox is at the 80% screen width.  Issues are that the buttons are not correctly resized to ensure that they appear correctly ( as is the case with the second messagebox ).  This is more down to fiddling with the button widths after the initial processing but will be fixed soon :o)


Credits
-------

Coded by Darren Owen, aka DrO (Sept 2003)

n0On3 - Idea request and helpful feedback ( the simplified function use over previous versions :o) )

A plugin from me to you :o)


Version History
-------

v0.1
* Initial release of the function
* Basic support to show that the button text can be altered on the messagebox
* Inital support for controlling the number of buttons shown

v0.5
* Setting of text of only the buttons wanted ( "/but? blah" )
* Now if a string is not passed then the text will not be set (broken in the v0.1 release)
* Altered a lot of internal workings to allow for more customising of the MessageBox
* Better support for the number of buttons to show
* The functional '4th' button - making use of the 'Help' button to act as a real button
* Fixed a number of crash issues due to buffer sizes and a few other bugs, etc

v0.8
* A lot of changes made :o)
* Improved internal handling so that now button text is set correctly either when the number of buttons is set or user styles are set so now "/but2 blah" will always set the second buttons text and not just a few cases as beforehand
* Altered the order of the parameters from earlier versions to make some internals easier to do :o)
* The 4th / Help button will now close the MessageBox in ALL cases - would not close correctly if there was not a 'cancel' button in earlier versions
* Buffers are now allocated to the NSIS buffer sizes passed to the dll on use for better compatibility between dll and installer
* When the button numbers are set, the function will map the returned value now to the button selected instead of the normal wndows defined return value
* some other bits and bobs that i've forgotten on the way

v0.9
* Added in support for displaying custom icons on the messagebox ( along with standard icons which can be shown ) either in the installer or from other files
* Altered the example script to show off some more bits of what it can do

v0.95 beta
* Added in support for resizing of buttons based on the text set into the button ( use the /trim before the text to be set )
* Messagebox is now limited to approximately 80% of the screen size ( as the messagebox normally does )
* The resizing of button texts will also update the width of the text width to ensure it keeps to the size of the messagebox
* Support is not complete for /trim ( still need to update the graduating of the width increment )
* When the 80% width is reached, currently buttons will disappear off the messagebox ( handling for a double height button will come to cope with this soon )
* Altered the example script to show off some more bits of what it can do

v0.98 beta
* Removed the passing in of a hwnd parameter - is always owned by the installer
* Altered the custom icon option to be a single parameter now in the form 'module,id' where module can be 0 to use the installer
* All return values are now mapped to the button number pressed ie 1,2,etc
* Now the number of buttons to show is based on the button string texts passed in to the function :o)
* Altered the order of parameters to work in a top to bottom order with respect to the messagebox layout and to match closer to the NSIS setup
[ a good suggestion from n0On3 - nice ]
- now goes:	style  title  usericon  text  but1  [ but2  but3  but4 ]
* Altered the button texts to be set in the order passed and will also convert the default ids too! eg IDYES -> "Yes", IDIGNORE -> "Ignore"
* Fixed the graduating of the button trim feature to appear 'natural'
[ adds approxiamately 3 character widths either side when the text won't fit fully in the button ]
* Fixed a few bugs in the code introduced in the changes made
* Support for MB_DEFBUTTON? is broken when a button has to be resized ( will fix this soon - already worked out how to but not why this happens )

v0.98 beta 2
* Fixed the MB_DEFBUTTON? issue ( needed to prevent the wm_active message being handled )
* Verified the installer will show icons correctly if the installer is run silently ( no code changes were made )
* Added in checking of the defined buttons to ensure that duplicates are not allowed (only the first occurrance is allowed)
* Checked that all strings are verified for having data before being used
* A few code tidy-ups introduced to the code
* Typo fix in the example
* Added in stripping out of the icon definitions if a user icon is specified (otherwise no icon will appear!)
* Updated the readme file to reflect the changes made to the plug-in's functionality since v0.98 beta and to clarify things in better detail now ( i hope )
* Added in a reference to the archive page for the plugin ( will upload majority of this file to the archive )
* All internal buffers are now set to the buffer size passed into the plugin when it is called and altered how they are referenced
* Fixed how the module string is cleared, should fix a potential crash issue i hope
* Partially fixed the resizing of the text area when using long button strings to correctly limit out when the 80% limit is reached ( fine when now icon is used at the moment )
* Added a silent installer option to the example script, just !define SILENT to get the silent version (should show that the function works fine either in a normal or silent installer)

v0.98 beta 3
* Fixed a crash when a silent installer is run ( pointed out by n0On3 ) - not checking the string length when getting the installer title if ""is the pased caption value, doh!
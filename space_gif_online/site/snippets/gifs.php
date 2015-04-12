
  <div class="row">

    <?php foreach (page('gifs')->images() as $image) : ?>
      <div class="col-md-3">
        <a href="<?php echo $image->url() ?>">
          <img src="<?php echo $image->url() ?>" alt="" class="img-responsive impb">
        </a>
      </div>
    <?php endforeach ?>

  </div>

